#pragma once

#include <type_traits>
#include <vector>

#include <stdint.h>

template <typename T>
inline constexpr size_t IntergerByteSize(T value)
{
    uint32_t clz = std::countl_zero(uint64_t(value));
    return static_cast<size_t>(((std::numeric_limits<T>::digits * 9 + 64) - (clz * 9)) / 64);
}

inline constexpr size_t TagSize(int number)
{
    uint64_t tag = number << 3;
    return IntergerByteSize(tag);
}

inline size_t LengthDelimitedSize(uint64_t size)
{
    return size + IntergerByteSize(size);
}

template <typename T>
inline constexpr size_t FixedByteSize(T value)
{
    return sizeof(value);
}

template <typename T>
inline constexpr size_t RepeatedFixedByteSize(const std::vector<T>& value)
{
    return FixedByteSize(value[0]) * value.size();
}

inline size_t StringSize(const std::string& value)
{
    return value.size() + IntergerByteSize(value.size());
}

template <typename T>
size_t RepeatIntergerByteSize(const std::vector<T>& value)
{
    size_t sz = 0;
    for (auto& v : value) {
        sz += IntergerByteSize(uint64_t(v));
    }
    return sz;
}
