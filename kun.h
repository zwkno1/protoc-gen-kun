#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <stdint.h>

namespace kun {

template <typename T, typename... Args>
inline constexpr bool is_one_of = std::disjunction<std::is_same<T, Args>...>::value;

// interger
template <typename T>
struct is_integral : std::integral_constant<bool, is_one_of<T, uint32_t, uint64_t, int32_t, int64_t>>
{
};

template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

// floating point
template <typename T>
struct is_floating_point : std::integral_constant<bool, is_one_of<T, float, double>>
{
};

template <typename T>
inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

// string
template <typename T>
struct is_string : std::integral_constant<bool, std::is_same_v<T, std::string>>
{
};

template <typename T>
inline constexpr bool is_string_v = is_string<T>::value;

// enum
template <typename T>
struct is_enum : std::false_type
{
};

template <typename T>
inline constexpr bool is_enum_v = is_enum<T>::value;

// message
template <typename T>
struct is_message : std::false_type
{
};

template <typename T>
inline constexpr bool is_message_v = is_message<T>::value;

// repeated
template <typename T>
struct is_repeated : std::false_type
{
};

template <typename T>
struct is_repeated<std::vector<T>> : std::true_type
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
struct is_map<std::unordered_map<K, V>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

// template <typename T>
// struct is_valid
//   : std::integral_constant<
//       bool, std::disjunction_v<is_integral<T>, is_floating_point<T>, is_string<T>, is_message<T>, is_enum<T>>>
//{
//     // constexpr static bool value =
// };
//
// template <typename K, typename V>
// struct is_valid<std::unordered_map<K, V>> : std::integral_constant<bool, std::disjunction_v<is_valid<K>,
// is_valid<V>>>
//{
// };
//
// template <typename T>
// struct is_valid<std::vector<T>> : std::integral_constant<bool, is_valid<T>::value>
//{
// };
//
// template <typename T>
// inline constexpr bool is_valid_v = is_valid<T>::value;

struct FieldMeta
{
    uint64_t number;
    uint64_t tag;
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

template <uint64_t number, typename T>
constexpr inline uint64_t MakeTag()
{
    uint32_t type;
    if constexpr (is_integral_v<T> || is_enum_v<T>) {
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

// is_integral_v<T> || is_floating_point_v<T> ||
//   std::is_same_v<T, std::string> || is_message<T>::value ||
//   (is_repeated<T>::value && is_valid_type<EntryType>::value) ||
//   (is_map<T>::value && is_valid_type<typename T::key_type>::value && is_valid_type<typename T::mapped_type>::value);

template <typename T>
inline constexpr size_t IntergerByteSize(T value)
{
    uint32_t clz = std::countl_zero(static_cast<uint64_t>(value));
    return static_cast<size_t>(((std::numeric_limits<uint64_t>::digits * 9 + 64) - (clz * 9)) / 64);
}

inline constexpr size_t TagSize(uint64_t number)
{
    uint64_t tag = number << 3;
    return IntergerByteSize(tag);
}

inline size_t LengthDelimitedSize(size_t size)
{
    return size + IntergerByteSize(size);
}

template <typename T>
    requires(std::is_unsigned_v<T>)
inline T EncodeInterger(T v)
{
    return v;
}

template <typename T>
    requires((std::is_signed_v<T> || is_enum_v<T>))
inline uint64_t EncodeInterger(T v)
{
    return static_cast<uint64_t>(v);
}

template <typename K, typename V>
struct MapEntry
{
    inline constexpr static std::array<FieldMeta, 2> __meta__ = {
        FieldMeta{ 1, MakeTag<1, K>(), "key" },
        FieldMeta{ 2, MakeTag<2, V>(), "value" },
    };

    template <typename Decoder>
    inline bool Decode(Decoder& dec, uint64_t tag)
    {
        switch (tag) {
        case __meta__[0].tag:
            return dec.Decode(entry_.first);
        case __meta__[1].tag:
            return dec.Decode(entry_.second);
        }
        return false;
    }

    // inline size_t ByteSize() const
    //{
    //     ByteSize<__meta__[0].number>(entry_.first) + ByteSize<__meta__[1].number>(entry_.second);
    // }

    std::pair<K, V>& entry_;
};

template <uint64_t Number, typename T>
inline size_t ByteSize(const T& value);

template <typename K, typename V>
struct ConstMapEntry
{
    inline constexpr static std::array<FieldMeta, 2> __meta__ = {
        FieldMeta{ 1, MakeTag<1, K>(), "key" },
        FieldMeta{ 2, MakeTag<2, V>(), "value" },
    };

    template <typename Encoder>
    void Encode(Encoder& enc) const
    {
        enc.Encode(__meta__[0], entry_.first);
        enc.Encode(__meta__[1], entry_.second);
    }

    inline size_t ByteSize() const
    {
        return ::kun::ByteSize<__meta__[0].number>(entry_.first) + ::kun::ByteSize<__meta__[1].number>(entry_.second);
    }

    const std::pair<const K, V>& entry_;
};

template <typename K, typename V>
struct is_message<MapEntry<K, V>> : std::true_type
{
};

template <typename K, typename V>
struct is_message<ConstMapEntry<K, V>> : std::true_type
{
};

// TODO: Check if T is valid type
template <uint64_t Number, typename T>
//    requires(is_valid_v<T>)
inline size_t ByteSize(const T& value)
{
    if constexpr (is_integral_v<T> || is_enum_v<T>) {
        return TagSize(Number) + IntergerByteSize(EncodeInterger(value));
    } else if constexpr (is_floating_point_v<T>) {
        return TagSize(Number) + sizeof(T);
    } else if constexpr (is_string_v<T>) {
        return TagSize(Number) + LengthDelimitedSize(value.size());
    } else if constexpr (is_message_v<T>) {
        auto size = value.ByteSize();
        if (size == 0) {
            return size;
        }
        return TagSize(Number) + LengthDelimitedSize(size);
    } else if constexpr (is_repeated_v<T>) {
        using EntryType = typename T::value_type;
        if (value.empty()) {
            return 0;
        }

        if constexpr (is_floating_point_v<EntryType>) {
            return TagSize(Number) + LengthDelimitedSize(sizeof(EntryType) * value.size());
        } else if constexpr (is_integral_v<EntryType> || is_enum_v<EntryType>) {
            size_t size = 0;
            for (auto& v : value) {
                size += IntergerByteSize(EncodeInterger(v));
            }
            return TagSize(Number) + LengthDelimitedSize(size);
        } else if constexpr (is_string_v<EntryType>) {
            size_t size = 0;
            for (auto& entry : value) {
                size += TagSize(Number) + LengthDelimitedSize(entry.size());
            }
            return size;
        } else if constexpr (is_message_v<EntryType>) {
            size_t size = 0;
            for (auto& entry : value) {
                size += TagSize(Number) + LengthDelimitedSize(entry.ByteSize());
            }
            return size;
        }
    } else if constexpr (is_map_v<T>) {
        using KeyType = typename T::key_type;
        using ValueType = typename T::mapped_type;
        size_t size = 0;
        for (auto& entry : value) {
            ConstMapEntry<KeyType, ValueType> e{ entry };
            auto entrySize = e.ByteSize();
            size += TagSize(Number) + LengthDelimitedSize(entrySize);
        }
        return size;
    }

    std::unreachable();
}

} // namespace kun