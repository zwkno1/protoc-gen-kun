#include <a.kun.h>
#include <bit>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include <codec.h>
#include <gtest/gtest.h>
#include <kun.h>

#include <b.pb.h>

std::mt19937_64 rng(std::random_device{}());

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

bool operator==(const kuntest::BBB& a, const pbtest::BBB& b);

template <typename T, typename U>
bool operator==(const std::vector<T>& a, const google::protobuf::RepeatedField<U>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

template <typename T, typename U>
bool operator==(const std::vector<T>& a, const google::protobuf::RepeatedPtrField<U>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); i++) {
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

bool operator==(const kuntest::AAA& a, const pbtest::AAA& b)
{

#define CHECK(name)                                                                                                    \
    do {                                                                                                               \
        if (a.name != b.name())                                                                                        \
            return false;                                                                                              \
    } while (0)

#define CHECK2(name)                                                                                                   \
    do {                                                                                                               \
        if (a.name != b.name())                                                                                        \
            return false;                                                                                              \
    } while (0)

    // CHECKREPEATED(a.name, b.name())

#define CHECK3(name)                                                                                                   \
    do {                                                                                                               \
        if (a.name.size() != b.name##_size()) {                                                                        \
            return false;                                                                                              \
        }                                                                                                              \
        for (auto& i : b.name()) {                                                                                     \
            auto iter = a.name.find(i.first);                                                                          \
            if (iter == a.name.end()) {                                                                                \
                return false;                                                                                          \
            }                                                                                                          \
            if (iter->second != i.second) {                                                                            \
                return false;                                                                                          \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

    CHECK(i32);
    CHECK(i64);
    CHECK(u32);
    CHECK(u64);
    CHECK(f);
    CHECK(d);
    CHECK(s);
    CHECK(b);
    CHECK(e);

    CHECK2(i32s);
    CHECK2(u32s);
    CHECK2(i64s);
    CHECK2(u64s);
    CHECK2(fs);
    CHECK2(ds);
    CHECK2(ss);
    CHECK2(bs);
    CHECK2(es);

    CHECK3(kvs);
    CHECK3(kvs2);

    CHECK2(bbbs);

    if (a.bbb.has_value() ^ b.has_bbb()) {
        return false;
    }

    if (a.bbb.has_value()) {
        if (a.bbb.value() != b.bbb()) {
            return false;
        }
    }

    return true;
}

pbtest::AAA ToPb(const kuntest::AAA& a)
{
    // int32 i32 = 112010;
    // uint32 u32 = 10111;
    // int64 i64 = 987112;
    // uint64 u64 = 684113;
    // float f = 35114;
    // double d = 2387115;
    // string s = 9116;
    // bytes b = 90117;
    // Error e = 2118;

    // repeated int32 i32s = 56210;
    // repeated uint32 u32s = 675211;
    // repeated int64 i64s = 2098712;
    // repeated uint64 u64s = 23123213;
    // repeated float fs = 123121124;
    // repeated double ds = 1321215;
    // repeated string ss = 21126;
    // repeated bytes bs = 1231217;
    // repeated Error es = 2181231;

    // map<int32, int32> kvs = 3;
    // map<int32, BBB> kvs2 = 4;
    // BBB bbb = 16;
    // repeated BBB bbbs = 123123;

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
    b.set_e(::pbtest::Error(a.e));

    COPY2(i32s);
    COPY2(u32s);
    COPY2(i64s);
    COPY2(u64s);
    COPY2(fs);
    COPY2(ds);
    COPY2(ss);
    COPY2(bs);
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

TEST(Encode, numeric)
{
    kuntest::AAA a;
    a.i32 = rng();
    a.u32 = rng();
    a.i64 = rng();
    a.u64 = rng();
    a.e = kuntest::Error(rng());
    a.f = std::bit_cast<float>(uint32_t(rng()));
    a.d = std::bit_cast<double>(rng());

    kun::Encoder enc;
    enc.Encode(a);

    pbtest::AAA b;
    EXPECT_TRUE(b.ParseFromString(enc.Str()));

    EXPECT_EQ(a.ByteSize(), b.ByteSizeLong());
    EXPECT_TRUE(a == b);
}

TEST(Decode, numeric)
{
    kuntest::AAA a;
    a.i32 = rng();
    a.u32 = rng();
    a.i64 = rng();
    a.u64 = rng();
    a.e = kuntest::Error(rng());
    a.f = std::bit_cast<float>(uint32_t(rng()));
    a.d = std::bit_cast<double>(rng());

    auto b = ToPb(a);
    auto str = b.SerializeAsString();

    kuntest::AAA c;

    kun::Decoder dec(str);
    EXPECT_TRUE(dec.Decode(c));

    EXPECT_TRUE(c == b);
}

TEST(Encode, map)
{
    kuntest::AAA a;

    for (int i = 0; i < 10; ++i) {
        a.kvs[rng()] = rng();
    }

    kun::Encoder enc;
    enc.Encode(a);

    pbtest::AAA b;
    EXPECT_TRUE(b.ParseFromString(enc.Str()));

    EXPECT_EQ(a.kvs.size(), b.kvs_size());

    for (auto& i : b.kvs()) {
        auto iter = a.kvs.find(i.first);
        EXPECT_TRUE(iter != a.kvs.end());
        EXPECT_EQ(iter->second, i.second);
    }
}

TEST(Encode, bytesize)
{
    kuntest::AAA a;

    a.i32 = 123123;
    a.f = 123.123;
    a.bbb = kuntest::BBB();
    for (int i = 0; i < 3000; i++) {
        a.ss.push_back(std::to_string(i));
        a.i32s.push_back(i * -12352);
        a.fs.push_back(i * 311.5);
        a.ds.push_back(i * 3231314.7);
        a.bbb->value.push_back(std::to_string(i * 37843));
        a.bbb->ints.push_back(i * 123131);
        a.es.push_back(kuntest::Error(i));
        a.kvs[i] = i * 1001;
    }

    pbtest::AAA b = ToPb(a);

    EXPECT_EQ(a.ByteSize(), b.ByteSizeLong());
}

TEST(Decode, base)
{
    kuntest::AAA a;
    a.u64 = std::numeric_limits<uint64_t>::max();
    a.d = 123.123;
    a.bbb = kuntest::BBB();
    for (int i = 0; i < 300000; i++) {
        a.ss.push_back(std::to_string(i));
        a.i32s.push_back(i * -12352);
        a.fs.push_back(i * 311.5);
        a.ds.push_back(i * 3231314.7);
        a.bbb->value.push_back(std::to_string(i * 37843));
        a.bbb->ints.push_back(i * 123131);
        a.es.push_back(kuntest::Error(i));
        a.kvs[i] = i * 1001;
    }

    auto b = ToPb(a);

    auto str = b.SerializeAsString();

    kuntest::AAA a2;
    kun::Decoder dec((uint8_t*)str.data(), str.size());

    EXPECT_TRUE(dec.Decode(a2));

    EXPECT_EQ(a, a2);
}