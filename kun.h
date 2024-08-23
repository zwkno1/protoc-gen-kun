#pragma once

#include <array>
#include <bit>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#define ALWAYS_INLINE inline //[[clang::always_inline]]

namespace kun {

template <typename T, typename... Args>
inline constexpr bool is_one_of = std::disjunction<std::is_same<T, Args>...>::value;

// bool
template <typename T>
struct is_boolean : std::integral_constant<bool, std::is_same_v<T, bool>>
{
};

template <typename T>
inline constexpr bool is_boolean_v = is_boolean<T>::value;

// interger
template <typename T>
struct is_integral : std::integral_constant<bool, is_one_of<T, uint32_t, uint64_t, int32_t, int64_t>>
{
};

template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// enum
template <typename T>
struct is_enum : std::false_type
{
};

template <typename T>
inline constexpr bool is_enum_v = is_enum<T>::value;

// floating point
template <typename T>
struct is_floating_point : std::integral_constant<bool, is_one_of<T, float, double>>
{
};

template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

// primitive
template <typename T>
struct is_primitive : std::disjunction<is_boolean<T>, is_integral<T>, is_enum<T>, is_floating_point<T>>
{
};

template <typename T>
inline constexpr bool is_primitive_v = is_primitive<T>::value;

// string
template <typename T>
struct is_string : std::integral_constant<bool, std::is_same_v<T, std::string>>
{
};

template <typename T>
inline constexpr bool is_string_v = is_string<T>::value;

// message
template <typename T>
struct is_message : std::false_type
{
};

template <typename T>
inline constexpr bool is_message_v = is_message<T>::value;

// sigular
template <typename T>
struct is_sigular : std::disjunction<std::disjunction<is_primitive<T>, is_string<T>, is_message<T>>>
{
};

template <typename T>
inline constexpr bool is_sigular_v = is_sigular<T>::value;

// repeated
template <typename T>
struct is_repeated : std::false_type
{
};

template <typename T>
struct is_repeated<std::vector<T>> : std::disjunction<is_sigular<T>>
{
};

template <typename T>
inline constexpr bool is_repeated_v = is_repeated<T>::value;

// map
template <typename T>
struct is_map : std::false_type
{
};

template <typename K, typename V>
struct is_map<std::unordered_map<K, V>>
  : std::conjunction<std::disjunction<is_primitive<K>, is_string<K>>, is_sigular<V>>
{
};

template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

// valid
// template <typename T>
// struct is_valid : std::disjunction<is_sigular<T>, is_repeated<T>, is_map<T>>
//{
//};

// template <typename T>
// inline constexpr bool is_valid_v = is_valid<T>::value;

enum EncodingType : uint8_t
{
    ENCODING_NONE = 0,
    ENCODING_VARINT = 1,
    ENCODING_ZIGZAG = 2,
    ENCODING_FIXED = 3,
    ENCODING_UTF8 = 4,
};
struct FieldMeta
{
    uint64_t number;
    uint64_t tag;
    uint32_t encoding;
    std::string_view name;
};

enum WireType : uint32_t
{
    WIRE_VARINT = 0,
    WIRE_FIXED64 = 1,
    WIRE_LENGTH_DELIM = 2,
    WIRE_START_GROUP = 3,
    WIRE_END_GROUP = 4,
    WIRE_FIXED32 = 5,
};

template <uint64_t number, uint32_t encoding, typename T>
//    requires(is_valid_v<T>)
inline constexpr uint64_t MakeTag()
{
    if constexpr (is_integral_v<T>) {
        if constexpr (encoding == ENCODING_FIXED) {
            if constexpr (sizeof(T) == 4) {
                return (number << 3) | WIRE_FIXED32;
            } else {
                return (number << 3) | WIRE_FIXED64;
            }
        }
        return (number << 3) | WIRE_VARINT;
    } else if constexpr (is_enum_v<T> || is_boolean_v<T>) {
        return (number << 3) | WIRE_VARINT;
    } else if constexpr (is_floating_point_v<T>) {
        if constexpr (sizeof(T) == 4) {
            return (number << 3) | WIRE_FIXED32;
        } else {
            return (number << 3) | WIRE_FIXED64;
        }
    }
    return (number << 3) | WIRE_LENGTH_DELIM;
}

template <uint32_t c = ENCODING_VARINT>
struct Encoding
{
    template <typename T>
        requires(std::is_unsigned_v<T>)
    static inline constexpr T Encode(T v)
    {
        return v;
    }

    template <typename T>
        requires(is_enum_v<T>)
    static inline constexpr uint64_t Encode(T v)
    {
        return static_cast<uint64_t>(v);
    }

    static inline constexpr uint64_t Encode(int32_t v)
    {
        if constexpr (c == ENCODING_ZIGZAG) {
            return (static_cast<uint32_t>(v) << 1) ^ static_cast<uint32_t>(v >> 31);
        }

        return static_cast<uint64_t>(v);
    }

    static inline constexpr uint64_t Encode(int64_t v)
    {
        if constexpr (c == ENCODING_ZIGZAG) {
            return (static_cast<uint64_t>(v) << 1) ^ static_cast<uint64_t>(v >> 63);
        }

        return static_cast<uint64_t>(v);
    }

    template <typename T>
    static inline constexpr T Decode(T value)
    {
        if (c == ENCODING_ZIGZAG) {
            return ((value >> 1) ^ (~(value & 1) + 1));
        }
        return value;
    }

    template <typename T>
    static inline constexpr size_t EncodedSize(T value)
    {
        if constexpr (c == ENCODING_FIXED) {
            return sizeof(T);
        }
        auto v = Encode(value);
        using EncodedType = decltype(v);
        uint32_t clz = std::countl_zero(v);
        return static_cast<size_t>(((std::numeric_limits<EncodedType>::digits * 9 + 64) - (clz * 9)) / 64);
    }
};

inline constexpr size_t TagSize(uint64_t tag)
{
    return Encoding<>::EncodedSize(tag);
}

inline constexpr size_t LengthDelimitedSize(uint64_t size)
{
    return size + Encoding<>::EncodedSize(size);
}

template <typename K, typename V, uint32_t encoding>
struct MapEntry
{
    using ThisType = MapEntry<K, V, encoding>;

    inline constexpr static std::array<FieldMeta, 2> __meta__ = {
        FieldMeta{ 1, MakeTag<1, (encoding >> 8), K>(), encoding >> 8, "key" },
        FieldMeta{ 2, MakeTag<2, (encoding & 0xFF), V>(), encoding & 0xFF, "value" },
    };

    template <typename Decoder>
    inline bool Decode(Decoder& dec, uint64_t tag)
    {
        switch (tag) {
        case __meta__[0].tag:
            return dec.template Decode<ThisType, 0>(entry_.first);
        case __meta__[1].tag:
            return dec.template Decode<ThisType, 1>(entry_.second);
        }
        return false;
    }

    std::pair<K, V>& entry_;
};

template <typename Msg, int index, typename T>
inline size_t ByteSizeWithTag(const T& value);

template <typename K, typename V, uint32_t encoding>
struct ConstMapEntry
{
    using ThisType = ConstMapEntry<K, V, encoding>;

    inline constexpr static std::array<FieldMeta, 2> __meta__ = {
        FieldMeta{ 1, MakeTag<1, (encoding >> 8), K>(), encoding >> 8, "key" },
        FieldMeta{ 2, MakeTag<2, (encoding & 0xFF), V>(), encoding & 0xFF, "value" },
    };

    template <typename Encoder>
    inline void Encode(Encoder& enc) const
    {
        enc.template Encode<ThisType, 0>(entry_.first);
        enc.template Encode<ThisType, 1>(entry_.second);
    }

    inline size_t ByteSize() const
    {
        //_cached_size_ =
        return ::kun::ByteSizeWithTag<ThisType, 0>(entry_.first) + ::kun::ByteSizeWithTag<ThisType, 0>(entry_.second);
        // return _cached_size_;
    }

    const std::pair<const K, V>& entry_;

    // mutable size_t _cached_size_;
};

template <typename K, typename V, uint32_t encoding>
struct is_message<MapEntry<K, V, encoding>> : std::true_type
{
};

template <typename K, typename V, uint32_t encoding>
struct is_message<ConstMapEntry<K, V, encoding>> : std::true_type
{
};

template <typename T>
// requires(is_valid_v<T>)
inline bool HasValue(const T& value)
{
    if constexpr (is_integral_v<T>) {
        return value != 0;
    } else if constexpr (is_boolean_v<T>) {
        return value;
    } else if constexpr (is_enum_v<T>) {
        return static_cast<int32_t>(value) != 0;
    } else if constexpr (is_floating_point_v<T>) {
        if constexpr (sizeof(T) == 8) {
            return *reinterpret_cast<const uint64_t*>(&value) != 0;
        } else if constexpr (sizeof(T) == 4) {
            return *reinterpret_cast<const uint32_t*>(&value) != 0;
        }
    } else if constexpr (is_string_v<T>) {
        return !value.empty();
    } else if constexpr (is_message_v<T>) {
        return value.ByteSize() != 0;
    } else if constexpr (is_repeated_v<T>) {
        return !value.empty();
    } else if constexpr (is_map_v<T>) {
        return !value.empty();
    }
    std::unreachable();
}

template <uint32_t encoding, typename T>
//    requires(is_valid_v<T>)
inline size_t ByteSize(const T& value)
{
    if constexpr (is_boolean_v<T>) {
        return 1;
    } else if constexpr (is_integral_v<T>) {
        return Encoding<encoding>::EncodedSize(value);
    } else if constexpr (is_enum_v<T>) {
        return Encoding<encoding>::EncodedSize(value);
    } else if constexpr (is_floating_point_v<T>) {
        return sizeof(T);
    } else if constexpr (is_string_v<T>) {
        return value.size();
    } else if constexpr (is_message_v<T>) {
        return value.ByteSize();
    } else if constexpr (is_repeated_v<T>) {
        using EntryType = typename T::value_type;
        if constexpr (is_boolean_v<EntryType>) {
            return value.size();
        } else if constexpr (is_floating_point_v<EntryType>) {
            return sizeof(EntryType) * value.size();
        } else if constexpr (is_integral_v<EntryType>) {
            if constexpr (encoding == ENCODING_FIXED) {
                return sizeof(EntryType) * value.size();
            } else {
                size_t size = 0;
                for (auto v : value) {
                    size += Encoding<encoding>::EncodedSize(v);
                }
                return size;
            }
        } else if constexpr (is_enum_v<EntryType>) {
            size_t size = 0;
            for (auto v : value) {
                size += Encoding<encoding>::EncodedSize(v);
            }
            return size;
        }
    }

    std::unreachable();
}

template <class Msg, int index, typename T>
// requires(is_valid_v<T>)
inline size_t ByteSizeWithTag(const T& value)
{
    constexpr auto meta = Msg::__meta__[index];
    constexpr auto tag = meta.tag;
    constexpr auto encoding = meta.encoding;
    if constexpr (is_primitive_v<T>) {
        return TagSize(tag) + ByteSize<encoding>(value);
    } else if constexpr (is_string_v<T> || is_message_v<T>) {
        return TagSize(tag) + LengthDelimitedSize(ByteSize<encoding>(value));
    } else if constexpr (is_repeated_v<T>) {
        using EntryType = typename T::value_type;
        if constexpr (is_primitive_v<EntryType>) {
            auto size = ByteSize<encoding>(value);
            return TagSize(tag) + LengthDelimitedSize(size);
        } else if constexpr (is_string_v<EntryType> || is_message_v<EntryType>) {
            size_t size = 0;
            for (auto& entry : value) {
                size += TagSize(tag) + LengthDelimitedSize(ByteSize<encoding>(entry));
            }
            return size;
        }
    } else if constexpr (is_map_v<T>) {
        using KeyType = typename T::key_type;
        using ValueType = typename T::mapped_type;
        size_t size = 0;
        for (auto& entry : value) {
            ConstMapEntry<KeyType, ValueType, encoding> e{ entry };
            size += TagSize(tag) + LengthDelimitedSize(ByteSize<encoding>(e));
        }
        return size;
    }

    std::unreachable();
}

} // namespace kun