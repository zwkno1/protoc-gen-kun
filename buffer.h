#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <kun.h>

namespace kun {

static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");

class Buffer
{
    enum WireType : uint32_t
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

    template <int number, typename T>
    void Write(const T& value)
    {
        if constexpr (is_integral_v<T>) {
            EnsureSpace();

            WriteTag<number, WIRETYPE_VARINT>();
            WriteVarint(Encode(value));
            return;
        } else if constexpr (is_floating_point_v<T>) {
            EnsureSpace();

            WriteTag<number, WIRETYPE_FIXED64>();
            WriteRaw(&value, 1);
            return;
        } else if constexpr (is_string_v<T>) {
            uint64_t size = value.size();
            EnsureSpace(size + 32);

            WriteLengthDelim<number>(size);
            WriteRaw(value.data(), size);
            return;
        } else if constexpr (is_message_v<T>) {
            auto size = value.ByteSize();
            if (size == 0) {
                return;
            }
            EnsureSpace(size + 32);
            WriteLengthDelim<number>(size);
            value.Serialize(*this);
            return;
        } else if constexpr (is_repeated<T>::value) {
            using ElementType = typename T::value_type;
            if constexpr (is_floating_point_v<ElementType>) {
                size_t size = sizeof(typename T::value_type) * value.size();
                EnsureSpace(size + 32);
                WriteLengthDelim<number>(size);
                WriteRaw(value.data(), value.size());
                return;
            } else if constexpr (is_integral_v<ElementType> || is_enum_v<ElementType>) {
                size_t size = 0;
                for (auto& v : value) {
                    size += IntergerByteSize(Encode(v));
                }

                EnsureSpace(size + 32);
                WriteLengthDelim<number>(size);
                for (auto v : value) {
                    WriteVarint(static_cast<uint64_t>(v));
                }
                return;
            }
        } else if constexpr (is_map_entry_v<T>) {
            auto size = TaggedByteSize<KeyFieldNumber>(value.first) + TaggedByteSize<ValueFieldNumber>(value.second);
            EnsureSpace(size + 32);
            WriteLengthDelim<number>(size);
            Write<KeyFieldNumber>(value.first);
            Write<ValueFieldNumber>(value.second);
            return;
        }
        std::unreachable();
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

    template <uint32_t number, uint32_t tag>
    void WriteTag()
    {
        WriteVarint((number << 3) | tag);
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

    template <int number>
    void WriteLengthDelim(uint32_t size)
    {
        WriteTag<number, WIRETYPE_LENGTH_DELIMITED>();
        WriteSize(size);
    }

    template <typename T>
    void WriteVarint(T value)
    {
        static_assert(std::is_unsigned_v<T>, "Varint serialization must be unsigned");
        while (value >= 0x80) [[unlikely]] {
            *ptr_ = static_cast<uint8_t>(value | 0x80);
            value >>= 7;
            ptr_++;
        }
        *ptr_ = static_cast<uint8_t>(value);
        ptr_++;
    }

    template <typename T>
    inline void WriteRaw(const T* src, size_t size)
    {
        if constexpr ((sizeof(T) != 1) && std::endian::native == std::endian::big) {
            for (auto end = src + size; src < end; src++) {
                T v = std::byteswap(*src);
                std::memcpy(ptr_, src, size * sizeof(T));
                ptr_ += sizeof(T);
            }
        } else {
            std::memcpy(ptr_, src, size * sizeof(T));
            ptr_ += size * sizeof(T);
        }
    }

private:
    uint8_t* data_;
    uint8_t* ptr_;
    size_t capacity_;
};

} // namespace kun