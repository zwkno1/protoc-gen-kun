#pragma once

#include <bit>
#include <cassert>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <kun.h>

namespace kun {

static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");

class Encoder
{

public:
    Encoder()
      : data_()
      , ptr_(reinterpret_cast<uint8_t*>(data_.data()))
    {
    }

    Encoder(const Encoder&) = delete;

    Encoder& operator=(const Encoder&) = delete;

    template <typename T>
        requires(is_message_v<T>)
    ALWAYS_INLINE void Encode(const T& value)
    {
        auto size = value.ByteSize();
        EnsureSpace(size);
        value.Encode(*this);
        assert(size == ptr_ - reinterpret_cast<uint8_t*>(data_.data()));
    }

    template <typename Msg, int index, typename T>
    ALWAYS_INLINE void Encode(const T& value, size_t cachedSize = 0)
    {
        constexpr auto meta = Msg::__meta__[index];
        if constexpr (is_boolean_v<T>) {
            EncodeTag(meta.tag);
            uint8_t v = value ? 1 : 0;
            EncodeRaw(&v, 1);
            return;
        } else if constexpr (is_integral_v<T>) {
            EncodeTag(meta.tag);

            if constexpr (meta.encoding == ENCODING_FIXED) {
                EncodeRaw(&value, 1);
            } else {
                EncodeVarint(Encoding<meta.encoding>::Encode(value));
            }
            return;
        } else if constexpr (is_enum_v<T>) {
            EncodeTag(meta.tag);
            EncodeVarint(Encoding<meta.encoding>::Encode(value));
            return;
        } else if constexpr (is_floating_point_v<T>) {
            EncodeTag(meta.tag);
            EncodeRaw(&value, 1);
            return;
        } else if constexpr (is_string_v<T>) {
            uint64_t size = value.size();

            EncodeLengthDelim(meta.tag, size);
            EncodeRaw(value.data(), size);
            return;
        } else if constexpr (is_message_v<T>) {
            auto size = value._cached_size_;
            EncodeLengthDelim(meta.tag, size);
            value.Encode(*this);
            return;
        } else if constexpr (is_repeated<T>::value) {
            using EntryType = typename T::value_type;
            if constexpr (is_boolean_v<EntryType>) {
                EncodeLengthDelim(meta.tag, value.size());
                for (auto b : value) {
                    uint8_t v = b ? 1 : 0;
                    EncodeRaw(&v, 1);
                }
                return;
            } else if constexpr (is_floating_point_v<EntryType>) {
                size_t size = sizeof(typename T::value_type) * value.size();
                EncodeLengthDelim(meta.tag, size);
                EncodeRaw(value.data(), value.size());
                return;
            } else if constexpr (is_integral_v<EntryType>) {
                if constexpr (meta.encoding == ENCODING_FIXED) {
                    size_t size = sizeof(EntryType) * value.size();
                    EncodeLengthDelim(meta.tag, size);
                    EncodeRaw(value.data(), value.size());
                } else {
                    size_t size = cachedSize;
                    EncodeLengthDelim(meta.tag, size);

                    for (auto v : value) {
                        EncodeVarint(Encoding<meta.encoding>::Encode(v));
                    }
                }
                return;
            } else if constexpr (is_enum_v<EntryType>) {
                size_t size = cachedSize;

                EncodeLengthDelim(meta.tag, size);
                for (auto v : value) {
                    EncodeVarint(Encoding<meta.encoding>::Encode(v));
                }
                return;
            } else if constexpr (is_string_v<EntryType>) {
                for (auto& entry : value) {
                    uint64_t size = entry.size();

                    EncodeLengthDelim(meta.tag, size);
                    EncodeRaw(entry.data(), size);
                }
                return;
            } else if constexpr (is_message_v<EntryType>) {
                for (auto& entry : value) {
                    auto size = entry._cached_size_;

                    EncodeLengthDelim(meta.tag, size);
                    entry.Encode(*this);
                }
                return;
            }
        } else if constexpr (is_map_v<T>) {
            using KeyType = typename T::key_type;
            using ValueType = typename T::mapped_type;
            for (auto& entry : value) {
                ConstMapEntry<KeyType, ValueType, meta.encoding> e{ entry };
                auto size = e.ByteSize();
                EncodeLengthDelim(meta.tag, size);
                e.Encode(*this);
            }
            return;
        }
        std::unreachable();
    }

    ALWAYS_INLINE std::string& Str() { return data_; }

private:
    ALWAYS_INLINE ALWAYS_INLINE void EnsureSpace(size_t size)
    {
        data_.resize(size);
        ptr_ = reinterpret_cast<uint8_t*>(data_.data());
    }

    ALWAYS_INLINE ALWAYS_INLINE void EncodeTag(uint64_t tag) { EncodeVarint(tag); }

    ALWAYS_INLINE ALWAYS_INLINE void EncodeLengthDelim(uint64_t tag, uint64_t size)
    {
        EncodeVarint(tag);
        EncodeVarint(size);
    }

    template <typename T>
        requires(std::is_unsigned_v<T>)
    ALWAYS_INLINE void EncodeVarint(T value)
    {
        while (value >= 0x80) [[unlikely]] {
            *ptr_++ = static_cast<uint8_t>(value | 0x80);
            value >>= 7;
        }
        *ptr_++ = static_cast<uint8_t>(value);
    }

    template <typename T>
    ALWAYS_INLINE ALWAYS_INLINE void EncodeRaw(const T* src, size_t size)
    {
        if constexpr ((sizeof(T) != 1) && std::endian::native == std::endian::big) {
            for (auto end = src + size; src < end; src++) {
                T v = std::byteswap(*src);
                std::memcpy(ptr_, &v, sizeof(T));
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

class Decoder
{
public:
    Decoder()
      : ptr_(nullptr)
      , end_(nullptr)
    {
    }

    Decoder(const std::string& str)
      : ptr_(reinterpret_cast<const uint8_t*>(str.data()))
      , end_(reinterpret_cast<const uint8_t*>(str.data()) + str.size())
    {
    }

    Decoder(const uint8_t* beg, uint64_t size)
      : ptr_(beg)
      , end_(beg + size)
    {
    }

    Decoder(const uint8_t* beg, const uint8_t* end)
      : ptr_(beg)
      , end_(end)
    {
    }

    template <typename T>
        requires(is_message_v<T>)
    ALWAYS_INLINE bool Decode(T& value)
    {
        while (ptr_ < end_) {
            auto [ok, tag] = DecodeVarint();

            if (!ok) {
                return false;
            }

            switch (tag & 0x07) {
            case WIRE_VARINT: {
                auto [ok, v] = DecodeVarint();
                if (!ok) {
                    return false;
                }

                Decoder d{ reinterpret_cast<uint8_t*>(&v), sizeof(v) };
                if (!value.Decode(d, tag)) {
                    return false;
                }
            } break;
            case WIRE_FIXED32: {
                if (Size() < 4) {
                    return false;
                }

                Decoder d{ ptr_, 4 };
                if (!value.Decode(d, tag)) {
                    return false;
                }
                ptr_ += 4;
            } break;
            case WIRE_FIXED64: {
                if (Size() < 8) {
                    return false;
                }

                Decoder d{ ptr_, 8 };
                if (!value.Decode(d, tag)) {
                    return false;
                }
                ptr_ += 8;
            } break;
            case WIRE_LENGTH_DELIM: {
                auto [ok, length] = DecodeLength();
                if (!ok) {
                    return false;
                }

                Decoder d{ ptr_, length };
                if (!value.Decode(d, tag)) {
                    return false;
                }
                ptr_ += length;
            } break;
            default:
                return false;
            }
        }
        return ptr_ == end_;
    }

    template <typename Msg, int index, typename T>
    ALWAYS_INLINE bool Decode(T& value)
    {
        constexpr auto meta = Msg::__meta__[index];
        if constexpr (is_boolean_v<T>) {
            value = (*reinterpret_cast<const uint64_t*>(ptr_) != 0);
            return true;
        } else if constexpr (is_integral_v<T>) {
            if constexpr (meta.encoding == ENCODING_FIXED) {
                if (!DecodeRaw(value)) {
                    return false;
                }
                return Empty();
            }
            uint64_t v = Encoding<meta.encoding>::Decode(*reinterpret_cast<const uint64_t*>(ptr_));

            if (!Convert(v, value)) {
                return false;
            }
            return true;
        } else if constexpr (is_enum_v<T>) {
            int32_t v;
            if (!Convert(*reinterpret_cast<const uint64_t*>(ptr_), v)) {
                return false;
            }

            value = static_cast<T>(v);
            return true;
        } else if constexpr (is_floating_point_v<T>) {
            if (!DecodeRaw(value)) {
                return false;
            }
            return Empty();
        } else if constexpr (is_string_v<T>) {
            value = std::string{ reinterpret_cast<const char*>(ptr_), reinterpret_cast<const char*>(end_) };
            return true;
        } else if constexpr (is_repeated<T>::value) {
            using EntryType = typename T::value_type;
            if constexpr (is_boolean_v<EntryType>) {
                value.resize(end_ - ptr_);
                for (size_t i = 0; i < value.size(); i++) {
                    value[i] = (*(ptr_ + i) != 0);
                }
                return true;
            } else if constexpr (is_floating_point_v<EntryType>) {
                if (!DecodeRaw(value)) {
                    return false;
                }

                return true;
            } else if constexpr (is_integral_v<EntryType>) {
                if constexpr (meta.encoding == ENCODING_FIXED) {
                    if (!DecodeRaw(value)) {
                        return false;
                    }

                    return true;
                } else {
                    while (ptr_ < end_) {
                        auto [ok, v] = DecodeVarint();
                        if (!ok) {
                            return false;
                        }

                        v = Encoding<meta.encoding>::Decode(v);

                        EntryType tmp;
                        if (!Convert(v, tmp)) {
                            return false;
                        }

                        value.push_back(tmp);
                    }
                    return Empty();
                }
            } else if constexpr (is_enum_v<EntryType>) {
                while (ptr_ < end_) {
                    auto [ok, v] = DecodeVarint();
                    if (!ok) {
                        return false;
                    }

                    int32_t tmp;
                    if (!Convert(v, tmp)) {
                        return false;
                    }

                    value.push_back(static_cast<EntryType>(tmp));
                }
                return Empty();
            }

            std::unreachable();
            return false;

        } else if constexpr (is_map_v<T>) {
            using KeyType = typename T::key_type;
            using ValueType = typename T::mapped_type;

            std::pair<KeyType, ValueType> v;
            MapEntry<KeyType, ValueType, meta.encoding> entry{ v };
            if (!this->template Decode(entry)) {
                return false;
            }
            value.emplace(std::move(v));
            return true;
        } else if constexpr (is_message_v<T>) {
            return Decode(value);
        }

        std::unreachable();
        return false;
    }

private:
    template <typename T = uint64_t>
    std::tuple<bool, T> DecodeVarint()
    {
        T value = 0;
        size_t n = 0;
        while ((ptr_ < end_) && (*ptr_ & 0x80)) {
            value |= ((T(*ptr_) & 0x7F) << (n * 7));
            ptr_++;
            n++;
        }

        if (ptr_ >= end_) {
            // truncated
            return { false, 0 };
        }

        value |= ((T(*ptr_) & 0x7F) << (n * 7));
        if (n * 7 + (8 - std::countl_zero(*ptr_)) > sizeof(T) * 8) {
            // overflow
            return { false, 0 };
        }

        ptr_++;
        return { true, value };
    }

    ALWAYS_INLINE std::tuple<bool, uint64_t> DecodeLength()
    {
        auto [ok, size] = DecodeVarint();
        if (!ok || Size() < size) {
            return { false, 0 };
        }
        return { ok, size };
    }

    template <typename T>
    ALWAYS_INLINE bool DecodeRaw(T& value)
    {
        if (sizeof(T) > Size()) {
            return false;
        }

        value = *reinterpret_cast<const T*>(ptr_);
        if constexpr (std::endian::native == std::endian::big) {
            value = std::byteswap(value);
        }
        ptr_ += sizeof(T);
        return true;
    }

    template <typename T>
    ALWAYS_INLINE bool DecodeRaw(std::vector<T>& value)
    {
        size_t size = end_ - ptr_;
        if (size % sizeof(T) != 0) {
            return false;
        }

        value.resize(size / sizeof(T));
        if constexpr (std::endian::native == std::endian::big) {
            for (auto& v : value) {
                v = std::byteswap(*reinterpret_cast<const T*>(ptr_));
                ptr_ += sizeof(T);
            }
        } else {
            std::memcpy(value.data(), ptr_, size);
        }
        ptr_ += sizeof(T);
        return true;
    }

    template <typename T>
    ALWAYS_INLINE bool Convert(uint64_t from, T& to)
    {
        if constexpr (std::is_signed_v<T>) {
            int64_t sv = std::bit_cast<int64_t>(from);
            if ((sv > std::numeric_limits<T>::max()) || (sv < std::numeric_limits<T>::min())) {
                // overflow
                return false;
            }
            to = from;
        } else {
            if (from > std::numeric_limits<T>::max()) {
                // overflow
                return false;
            }
            to = from;
        }
        return true;
    }

    ALWAYS_INLINE size_t Size() const { return end_ - ptr_; }
    ALWAYS_INLINE bool Empty() const { return end_ - ptr_ == 0; }

    const uint8_t* ptr_;
    const uint8_t* end_;
};

} // namespace kun