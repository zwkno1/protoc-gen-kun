#pragma once

#include <bit>
#include <cstddef>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <_types/_uint64_t.h>
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
struct is_map<std::map<K, V>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_map_v = is_map<T>::value;

// map entry
template <typename T>
struct is_map_entry : std::false_type
{
};

template <typename K, typename V>
struct is_map_entry<std::pair<K, V>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_map_entry_v = is_map_entry<T>::value;

// is_integral_v<T> || is_floating_point_v<T> ||
//   std::is_same_v<T, std::string> || is_message<T>::value ||
//   (is_repeated<T>::value && is_valid_type<typename T::value_type>::value) ||
//   (is_map<T>::value && is_valid_type<typename T::key_type>::value && is_valid_type<typename T::mapped_type>::value);

inline constexpr int KeyFieldNumber = 1;
inline constexpr int ValueFieldNumber = 2;

template <typename T>
inline constexpr size_t IntergerByteSize(T value)
{
    uint32_t clz = std::countl_zero(static_cast<uint64_t>(value));
    return static_cast<size_t>(((std::numeric_limits<uint64_t>::digits * 9 + 64) - (clz * 9)) / 64);
}

inline constexpr size_t TagSize(int number)
{
    uint64_t tag = number << 3;
    return IntergerByteSize(tag);
}

inline size_t LengthDelimitedSize(size_t size)
{
    return size + IntergerByteSize(size);
}

template <typename T>
    requires((std::is_unsigned_v<T>))
inline T Encode(T v)
{
    return v;
}

template <typename T>
    requires((std::is_signed_v<T> || is_enum_v<T>))
inline uint64_t Encode(T v)
{
    return static_cast<uint64_t>(v);
}

// T must not be repeated message.
// TODO: Check if T is valid type
template <typename T>
inline size_t ByteSize(const T& value)
{
    if constexpr (is_integral_v<T>) {
        return IntergerByteSize(Encode(value));
    } else if constexpr (is_floating_point_v<T>) {
        return sizeof(T);
    } else if constexpr (is_string_v<T>) {
        return LengthDelimitedSize(value.size());
    } else if constexpr (is_enum_v<T>) {
        return IntergerByteSize(Encode(value));
    } else if constexpr (is_message_v<T>) {
        return value.ByteSize();
    } else if constexpr (is_repeated_v<T>) {
        if (is_floating_point_v<typename T::value_type>) {
            return LengthDelimitedSize(sizeof(typename T::value_type) * value.size());
        } else if constexpr (is_integral_v<typename T::value_type> || is_enum_v<typename T::value_type>) {
            size_t size = 0;
            for (auto& v : value) {
                size += IntergerByteSize(Encode(v));
            }
            return LengthDelimitedSize(size);
        }
    } else if constexpr (is_map_entry_v<T>) {
        auto size = TagSize(1) + ByteSize(value.first) + TagSize(2) + ByteSize(value.second);
        return LengthDelimitedSize(size);
    }

    std::unreachable();
}

// T must not be repeated message.
template <int number, typename T>
inline size_t TaggedByteSize(const T& value)
{
    return TagSize(number) + ByteSize(value);
}

template <typename Key, typename Value>
class MapEntry
{

    Key& k_;
    Value& v_;
};

} // namespace kun