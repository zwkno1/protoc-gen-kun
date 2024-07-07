#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#include <util.h>

static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");

class Buffer
{
    enum WireType : int
    {
        WIRETYPE_VARINT = 0,
        WIRETYPE_FIXED64 = 1,
        WIRETYPE_LENGTH_DELIMITED = 2,
        WIRETYPE_START_GROUP = 3,
        WIRETYPE_END_GROUP = 4,
        WIRETYPE_FIXED32 = 5,
    };

public:
    Buffer()
      : data_(nullptr)
      , ptr_(nullptr)
      , capacity_(0)
    {
    }

    Buffer(const Buffer&) = delete;

    Buffer& operator=(const Buffer&) = delete;

    ~Buffer() { delete[] data_; }

    template <typename T>
    void WriteRepeatedInterger(int num, const std::vector<T>& values)
    {
        size_t size = RepeatIntergerByteSize(values);
        EnsureSpace(size + 32);
        WriteLengthDelim(num, size);
        for (auto& v : values) {
            WriteVarint(uint64_t(v));
        }
    }

    template <typename T>
    void WriteRepeatedFixed(int num, const std::vector<T>& values)
    {
        size_t size = RepeatedFixedByteSize(values);
        EnsureSpace(size + 32);
        WriteLengthDelim(num, size);
        if constexpr (std::endian::native == std::endian::big) {
            for (auto v : values) {
                v = std::byteswap(v);
                WriteRaw(&v, sizeof(v));
            }
        } else {
            WriteRaw(values.data(), size);
        }
    }

    template <typename T>
    void WriteInterger(int num, T value)
    {
        uint64_t v = static_cast<uint64_t>(value);
        EnsureSpace();
        WriteTag<WIRETYPE_VARINT>(num);
        WriteVarint(v);
    }

    template <typename T>
    inline void WriteFixed(int num, T value)
    {
        EnsureSpace();
        WriteTag<WIRETYPE_FIXED64>(num);
        if constexpr (std::endian::native == std::endian::big) {
            value = std::byteswap(value);
        }
        WriteRaw(&value, sizeof(value));
    }

    inline void WriteString(int num, const std::string& value)
    {
        uint64_t size = value.size();
        EnsureSpace(size + 32);

        WriteLengthDelim(num, size);
        WriteRaw(value.data(), size);
    }

    template <typename T>
    inline void WriteMessage(int num, const T& value)
    {
        uint64_t size = value.ByteSize();
        EnsureSpace(size + 32);
        WriteLengthDelim(num, size);
        value.Serialize(*this);
    }

    inline void WriteEnum(int num, int value) { WriteInterger(num, value); }

    template <typename T>
    inline void WriteMap(int num, const T& value)
    {
        EnsureSpace();
        WriteTag<WIRETYPE_LENGTH_DELIMITED>(num);
    }

    inline uint8_t* Data() const { return data_; }
    inline size_t Size() const { return ptr_ - data_; }

private:
    void EnsureSpace(uint32_t size = 32)
    {
        auto s = ptr_ - data_;
        if (s + size > capacity_) {
            capacity_ = std::max<size_t>(std::max<size_t>(capacity_ * 2, 32), s + size);
            uint8_t* data = new uint8_t[capacity_];
            std::memcpy(data, data_, s);
            delete[] data_;
            data_ = data;
            ptr_ = data_ + s;
        }
    }

    template <int tag>
    void WriteTag(uint32_t num)
    {
        WriteVarint((num << 3) | tag);
    }

    void WriteSize(uint32_t value)
    {
        while (value >= 0x80) {
            *ptr_ = static_cast<uint8_t>(value | 0x80);
            value >>= 7;
            ++ptr_;
        }
        *ptr_++ = static_cast<uint8_t>(value);
    }

    void WriteLengthDelim(int num, uint32_t size)
    {
        WriteTag<WIRETYPE_LENGTH_DELIMITED>(num);
        WriteSize(size);
    }

    template <typename T>
    void WriteVarint(T value)
    {
        static_assert(std::is_unsigned_v<T>, "Varint serialization must be unsigned");
        while (value >= 0x80) [[unlikely]] {
            *ptr_ = static_cast<uint8_t>(value | 0x80);
            value >>= 7;
            ++ptr_;
        }
        *ptr_++ = static_cast<uint8_t>(value);
    }

    inline void WriteRaw(const void* data, size_t size)
    {
        std::memcpy(ptr_, data, size);
        ptr_ += size;
    }

private:
    uint8_t* data_;
    uint8_t* ptr_;
    size_t capacity_;
};