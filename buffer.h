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
        WIRE_VARINT = 0,
        WIRE_FIXED64 = 1,
        WIRE_LENGTH_DELIM = 2,
        // WIRE_START_GROUP = 3,
        // WIRE_END_GROUP = 4,
        WIRE_FIXED32 = 5,
    };

public:
    Buffer()
      : data_()
      , ptr_(reinterpret_cast<uint8_t*>(data_.data()))
    {
    }

    Buffer(const Buffer&) = delete;

    Buffer& operator=(const Buffer&) = delete;

    ~Buffer() {}

    template <typename T>
    void Write(const FieldMeta& meta, const T& value)
    {
        if constexpr (is_integral_v<T> || is_enum_v<T>) {
            EnsureSpace();

            WriteTag(meta.number, WIRE_VARINT);
            WriteVarint(Encode(value));
            return;
        } else if constexpr (is_floating_point_v<T>) {
            EnsureSpace();

            if constexpr (sizeof(T) == 4) {
                WriteTag(meta.number, WIRE_FIXED32);
            } else if constexpr (sizeof(T) == 8) {
                WriteTag(meta.number, WIRE_FIXED64);
            }

            WriteRaw(&value, 1);
            return;
        } else if constexpr (is_string_v<T>) {
            uint64_t size = value.size();
            EnsureSpace(size);

            WriteLengthDelim(meta.number, size);
            WriteRaw(value.data(), size);
            return;
        } else if constexpr (is_message_v<T>) {
            auto size = value.ByteSize();
            if (size == 0) {
                return;
            }
            EnsureSpace(size);
            WriteLengthDelim(meta.number, size);
            value.Serialize(*this);
            return;
        } else if constexpr (is_repeated<T>::value) {
            if (value.empty()) {
                return;
            }

            using EntryType = typename T::value_type;
            if constexpr (is_floating_point_v<EntryType>) {
                size_t size = sizeof(typename T::value_type) * value.size();
                EnsureSpace(size);
                WriteLengthDelim(meta.number, size);
                WriteRaw(value.data(), value.size());
                return;
            } else if constexpr (is_integral_v<EntryType> || is_enum_v<EntryType>) {
                size_t size = 0;
                for (auto& v : value) {
                    size += IntergerByteSize(Encode(v));
                }

                EnsureSpace(size);
                WriteLengthDelim(meta.number, size);
                for (auto& v : value) {
                    WriteVarint(Encode(v));
                }
                return;
            } else if constexpr (is_string_v<EntryType>) {
                for (auto& entry : value) {
                    uint64_t size = entry.size();
                    EnsureSpace(size);

                    WriteLengthDelim(meta.number, size);
                    WriteRaw(entry.data(), size);
                }
                return;
            } else if constexpr (is_message_v<EntryType>) {
                for (auto& entry : value) {
                    auto size = entry.ByteSize();
                    EnsureSpace(size);
                    WriteLengthDelim(meta.number, size);
                    entry.Serialize(*this);
                }
                return;
            }
        } else if constexpr (is_map_v<T>) {
            for (auto& entry : value) {
                auto size = ByteSize<KeyFieldMeta.number>(entry.first) + ByteSize<ValueFieldMeta.number>(entry.second);
                EnsureSpace(size);
                WriteLengthDelim(meta.number, size);
                Write(KeyFieldMeta, entry.first);
                Write(ValueFieldMeta, entry.second);
            }
            return;
        }
        std::unreachable();
    }

    inline std::string& Str()
    {
        data_.resize(Size());
        return data_;
    }

    inline size_t Size() const { return ptr_ - reinterpret_cast<const uint8_t*>(data_.data()); }

private:
    static inline constexpr uint64_t MakeTag(uint64_t number, uint32_t tag) { return (number << 3) | tag; }

    void EnsureSpace(uint32_t size = 0)
    {
        size += 32;
        auto oldSize = Size();
        if (oldSize + size > data_.size()) {
            data_.resize(oldSize + size);
            ptr_ = reinterpret_cast<uint8_t*>(data_.data()) + oldSize;
        }
    }

    inline void WriteTag(uint64_t number, uint32_t tag)
    {
        auto t = MakeTag(number, tag);
        WriteVarint(t);
    }

    inline void WriteLengthDelim(uint64_t number, uint32_t size)
    {
        WriteTag(number, WIRE_LENGTH_DELIM);
        WriteVarint(size);
    }

    template <typename T>
    inline void WriteVarint(T value)
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
    std::string data_;
    uint8_t* ptr_;
};

} // namespace kun