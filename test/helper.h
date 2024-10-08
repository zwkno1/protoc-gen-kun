#pragma once

#include <a.kun.h>
#include <bit>
#include <random>

#include <codec.h>
#include <gtest/gtest.h>
#include <kun.h>

#include <b.pb.h>

void Print(const std::string& data)
{
    std::stringstream ss;
    for (size_t i = 0; i < data.size(); i++) {
        if (i % 16 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex << ((uint32_t)data[i] & 0xFF) << " ";
    }
    std::cout << ss.str() << std::endl;
}

template <typename T>
bool FloatEqual(T a, T b)
{
    if constexpr (sizeof(T) == 8) {
        EXPECT_TRUE(std::bit_cast<uint64_t>(a) == std::bit_cast<uint64_t>(b));
        return std::bit_cast<uint64_t>(a) == std::bit_cast<uint64_t>(b);
    } else if constexpr (sizeof(T) == 4) {
        EXPECT_TRUE(std::bit_cast<uint32_t>(a) == std::bit_cast<uint32_t>(b));
        return std::bit_cast<uint32_t>(a) == std::bit_cast<uint32_t>(b);
    }
    return false;
}

bool operator==(const kuntest::BBB& a, const pbtest::BBB& b);

template <typename T, typename U>
bool operator==(const std::vector<T>& a, const google::protobuf::RepeatedField<U>& b)
{
    EXPECT_EQ(a.size(), b.size());
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); i++) {
        if constexpr (kun::is_floating_point_v<T>) {
            return FloatEqual(a[i], b[i]);
        } else {
            if constexpr (kun::is_integral_v<T> || kun::is_string_v<T>) {
                EXPECT_EQ(a[i], b[i]);
                if (a[i] != b[i]) {
                    return false;
                }
            } else if constexpr (kun::is_enum_v<T>) {
                EXPECT_TRUE(static_cast<int32_t>(a[i]) == static_cast<int32_t>(b[i]));
                if (static_cast<int32_t>(a[i]) != static_cast<int32_t>(b[i])) {
                    return false;
                }
            } else {
                EXPECT_TRUE(a[i] == b[i]);

                if (a[i] != b[i]) {
                    return false;
                }
            }
        }
    }
    return true;
}

template <typename T, typename U>
bool operator==(const std::vector<T>& a, const google::protobuf::RepeatedPtrField<U>& b)
{
    EXPECT_EQ(a.size(), b.size());
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); i++) {
        if constexpr (kun::is_floating_point_v<T>) {
            return FloatEqual(a[i], b[i]);
        } else if constexpr (kun::is_integral_v<T> || kun::is_string_v<T>) {
            EXPECT_EQ(a[i], b[i]);
        } else {
            EXPECT_TRUE(a[i] == b[i]);
        }
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

bool operator==(const kuntest::BBB& a, const pbtest::BBB& b)
{
    return (a.value == b.value()) && (a.ints == b.ints());
}

template <typename K1, typename V1, typename K2, typename V2>
bool operator==(const std::unordered_map<K1, V1>& a, const google::protobuf::Map<K2, V2>& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (auto& e : b) {
        auto iter = a.find(e.first);
        if (iter == a.end()) {
            return false;
        }
        if (iter->second != e.second) {
            return false;
        }
    }
    return true;
}

bool operator==(const kuntest::AAA& a, const pbtest::AAA& b)
{

#define CHECK(name)                                                                                                    \
    do {                                                                                                               \
        if (a.name != b.name())                                                                                        \
            return false;                                                                                              \
    } while (0)

#define CHECK2(name)                                                                                                   \
    do {                                                                                                               \
        if (a.name != static_cast<decltype(a.name)>(b.name())) {                                                       \
            return false;                                                                                              \
        }                                                                                                              \
    } while (0)

    CHECK(i32);
    CHECK(i64);
    CHECK(u32);
    CHECK(u64);

    CHECK(s32);
    CHECK(s64);
    CHECK(sf32);
    CHECK(sf64);
    CHECK(f32);
    CHECK(f64);

    CHECK(s);
    CHECK(bt);
    CHECK2(e);
    CHECK(b);

    if (!FloatEqual(a.f, b.f())) {
        return false;
    }

    if (!FloatEqual(a.d, b.d())) {
        return false;
    }

    CHECK(i32s);
    CHECK(u32s);
    CHECK(i64s);
    CHECK(u64s);
    CHECK(fs);
    CHECK(ds);
    CHECK(ss);
    CHECK(bts);
    CHECK(es);
    CHECK(bs);

    CHECK(s32s);
    CHECK(s64s);
    CHECK(sf32s);
    CHECK(sf64s);
    CHECK(f32s);
    CHECK(f64s);

    CHECK(kvs);
    CHECK(kvs2);

    CHECK(bbbs);

    if ((!!a.bbb) != b.has_bbb()) {
        return false;
    }

    if (a.bbb) {
        if ((!!a.bbb) != b.has_bbb()) {
            return false;
        }
    }

    return true;
}

void ExpectEQ(const kuntest::AAA& a, const pbtest::AAA& b)
{
    EXPECT_EQ(a.i32, b.i32());
    EXPECT_TRUE(a.i64 == b.i64());
    EXPECT_TRUE(a.u32 == b.u32());
    EXPECT_TRUE(a.u64 == b.u64());
    EXPECT_TRUE(std::bit_cast<uint32_t>(a.f) == std::bit_cast<uint32_t>(b.f()));
    EXPECT_TRUE(std::bit_cast<uint64_t>(a.d) == std::bit_cast<uint64_t>(b.d()));
    EXPECT_TRUE(a.s == b.s());
    EXPECT_TRUE(a.bt == b.bt());
    EXPECT_TRUE(a.e == kuntest::Error(b.e()));
    EXPECT_TRUE(a.b == b.b());

    EXPECT_TRUE(a.s32 == b.s32());
    EXPECT_TRUE(a.s64 == b.s64());
    EXPECT_TRUE(a.sf32 == b.sf32());
    EXPECT_TRUE(a.sf64 == b.sf64());
    EXPECT_TRUE(a.f32 == b.f32());
    EXPECT_TRUE(a.f64 == b.f64());

    EXPECT_TRUE(a.i32s == b.i32s());
    EXPECT_TRUE(a.u32s == b.u32s());
    EXPECT_TRUE(a.i64s == b.i64s());
    EXPECT_TRUE(a.u64s == b.u64s());
    EXPECT_TRUE(a.fs == b.fs());
    EXPECT_TRUE(a.ds == b.ds());
    EXPECT_TRUE(a.ss == b.ss());
    EXPECT_TRUE(a.bts == b.bts());
    EXPECT_TRUE(a.es == b.es());
    EXPECT_TRUE(a.bs == b.bs());

    EXPECT_TRUE(a.s32s == b.s32s());
    EXPECT_TRUE(a.s64s == b.s64s());
    EXPECT_TRUE(a.sf32s == b.sf32s());
    EXPECT_TRUE(a.sf64s == b.sf64s());
    EXPECT_TRUE(a.f32s == b.f32s());
    EXPECT_TRUE(a.f64s == b.f64s());

    EXPECT_TRUE(a.kvs == b.kvs());
    EXPECT_TRUE(a.kvs2 == b.kvs2());

    EXPECT_TRUE(a.bbbs == b.bbbs());

    EXPECT_TRUE((!!a.bbb) == b.has_bbb());
    if (a.bbb) {
        EXPECT_TRUE(*a.bbb == b.bbb());
    }
}

pbtest::AAA ToPb(const kuntest::AAA& a)
{

#define COPY(name) b.set_##name(a.name)

#define COPY2(name)                                                                                                    \
    do {                                                                                                               \
        for (auto& i : a.name) {                                                                                       \
            b.add_##name(i);                                                                                           \
        }                                                                                                              \
    } while (0)

#define COPY3(name)                                                                                                    \
    do {                                                                                                               \
        for (auto& i : a.name) {                                                                                       \
            (*b.mutable_##name())[i.first] = i.second;                                                                 \
        }                                                                                                              \
    } while (0)

    pbtest::AAA b;
    COPY(i32);
    COPY(i64);
    COPY(u32);
    COPY(u64);
    COPY(f);
    COPY(d);
    COPY(s);
    COPY(b);
    COPY(bt);
    b.set_e(::pbtest::Error(a.e));

    COPY(s32);
    COPY(s64);
    COPY(sf32);
    COPY(sf64);
    COPY(f32);
    COPY(f64);

    COPY2(i32s);
    COPY2(u32s);
    COPY2(i64s);
    COPY2(u64s);
    COPY2(fs);
    COPY2(ds);
    COPY2(ss);
    COPY2(bts);

    COPY2(s32s);
    COPY2(s64s);
    COPY2(sf32s);
    COPY2(sf64s);
    COPY2(f32s);
    COPY2(f64s);

    for (auto i : a.bs) {
        b.add_bs(i);
    }

    for (auto& i : a.es) {
        b.add_es(::pbtest::Error(i));
    }

    COPY3(kvs);

    for (auto& i : a.kvs2) {
        pbtest::BBB bbb;

        for (auto& j : i.second.ints) {
            bbb.add_ints(j);
        }

        for (auto& j : i.second.value) {
            bbb.add_value(j);
        }

        (*b.mutable_kvs2())[i.first] = bbb;
    }

    if (a.bbb) {
        pbtest::BBB& bbb = *b.mutable_bbb();

        for (auto& j : a.bbb->ints) {
            bbb.add_ints(j);
        }

        for (auto& j : a.bbb->value) {
            bbb.add_value(j);
        }
    }

    for (auto& i : a.bbbs) {
        pbtest::BBB& bbb = *b.add_bbbs();

        for (auto& j : i.ints) {
            bbb.add_ints(j);
        }

        for (auto& j : i.value) {
            bbb.add_value(j);
        }
    }

    EXPECT_TRUE(a == b);

    return b;
}

inline uint64_t rng()
{
    static std::mt19937_64 r(std::random_device{}());
    return r();
}

template <typename T>
void GenRand(T& value)
{
    if constexpr (std::is_same_v<T, float>) {
        value = std::bit_cast<float>(uint32_t(rng()));
    } else if constexpr (std::is_same_v<T, double>) {
        value = std::bit_cast<double>(rng());
    } else if constexpr (std::is_same_v<T, bool>) {
        value = rng() % 2;
    } else if constexpr (kun::is_string_v<T>) {
        value.resize(rng() % 100);
        for (auto& i : value) {
            i = rng() % 128;
        }
    } else if constexpr (kun::is_integral_v<T>) {
        auto v = rng();
        value = *reinterpret_cast<T*>(&v);
    }
}

template <typename T>
void GenRandRepeated(std::vector<T>& value, size_t max = 1000)
{
    size_t size = rng() % max;
    for (size_t i = 0; i < size; i++) {
        T v;
        GenRand(v);
        value.push_back(std::move(v));
    }
}

inline kuntest::AAA GenAAA()
{
    kuntest::AAA a;

    GenRand(a.i32);
    GenRand(a.u32);
    GenRand(a.i64);
    GenRand(a.u64);
    GenRand(a.e);
    GenRand(a.f);
    GenRand(a.d);
    GenRand(a.s);
    GenRand(a.bt);
    GenRand(a.b);

    GenRand(a.s32);
    GenRand(a.s64);
    GenRand(a.sf32);
    GenRand(a.sf64);
    GenRand(a.f32);
    GenRand(a.f64);

    GenRandRepeated(a.i32s);
    GenRandRepeated(a.u32s);
    GenRandRepeated(a.i64s);
    GenRandRepeated(a.u64s);
    GenRandRepeated(a.es);
    GenRandRepeated(a.fs);

    GenRandRepeated(a.ds);
    GenRandRepeated(a.ss);
    GenRandRepeated(a.bts);
    GenRandRepeated(a.bs);

    GenRandRepeated(a.s32s);
    GenRandRepeated(a.s64s);
    GenRandRepeated(a.sf32s);
    GenRandRepeated(a.sf64s);
    GenRandRepeated(a.f32s);
    GenRandRepeated(a.f64s);

    for (int i = 0; i < rng() % 100; i++) {
        a.kvs[rng()] = rng();
    }

    if (rng() % 2 == 0) {
        a.bbb = std::make_unique<::kuntest::BBB>();
        GenRandRepeated(a.bbb->ints);
        GenRandRepeated(a.bbb->value);
    }

    for (int i = 0; i < rng() % 100; i++) {
        std::string k;
        GenRand(k);
        kuntest::BBB v;
        GenRandRepeated(v.ints);
        GenRandRepeated(v.value);
        a.kvs2.emplace(std::move(k), std::move(v));
    }

    return a;
}