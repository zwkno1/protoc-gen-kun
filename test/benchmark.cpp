
#include <a.kun.h>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include <codec.h>
#include <kun.h>

#include <b.pb.h>

std::mt19937_64 rng(std::random_device{}());

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

    return b;
}

int main(int argc, char* argv[])
{
    kuntest::AAA a;
    a.i32 = rng();
    a.u32 = rng();
    a.i64 = rng();
    a.u64 = rng();
    a.e = kuntest::Error(rng());
    a.f = std::bit_cast<float>(uint32_t(rng()));
    a.d = std::bit_cast<double>(rng());

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

    {

        auto start = std::chrono::steady_clock::now();
        size_t size;
        for (int i = 0; i < 10000; i++) {
            kun::Encoder enc;
            enc.Encode(a);
            size += enc.Str().size();
        }
        auto end = std::chrono::steady_clock::now();
        std::cout << "cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms, bytes: " << size << std::endl;
    }

    {

        auto start = std::chrono::steady_clock::now();
        size_t size;
        for (int i = 0; i < 10000; i++) {
            size += b.SerializeAsString().size();
        }
        auto end = std::chrono::steady_clock::now();
        std::cout << "cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms, bytes: " << size << std::endl;
    }

    return 0;
}
