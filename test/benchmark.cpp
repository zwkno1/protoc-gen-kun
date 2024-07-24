
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include <google/protobuf/json/json.h>
#include <gperftools/profiler.h>

#include "a.kun.h"
#include "b.pb.h"
#include "codec.h"
#include "test/helper.h"

int main(int argc, char* argv[])
{
    int n = 100;
    if (argc >= 2) {
        n = atoi(argv[1]);
    }
    std::cout << "benchmark n: " << n << std::endl;

    kuntest::AAA a = GenAAA();
    // a.i32 = rng();
    // a.u32 = rng();
    // a.i64 = rng();
    // a.u64 = rng();
    // a.e = kuntest::Error(rng());
    // a.f = std::bit_cast<float>(uint32_t(rng()));
    // a.d = std::bit_cast<double>(rng());

    // a.i32 = 123123;
    // a.f = 123.123;
    // a.bbb = kuntest::BBB();
    // for (int i = 0; i < 3000; i++) {
    //     a.ss.push_back(std::to_string(i));
    //     a.i32s.push_back(i * -12352);
    //     a.fs.push_back(i * 311.5);
    //     a.ds.push_back(i * 3231314.7);
    //     a.bbb->value.push_back(std::to_string(i * 37843));
    //     a.bbb->ints.push_back(i * 123131);
    //     a.es.push_back(kuntest::Error(i));
    //     a.kvs[i] = i * 1001;
    // }
    pbtest::AAA b = ToPb(a);
    std::string s = b.SerializeAsString();

#if 1
    {
        auto start = std::chrono::steady_clock::now();
        // ProfilerStart("bench.prof");
        size_t size = 0;
        for (int i = 0; i < n; i++) {
            kun::Encoder enc;
            enc.Encode(a);
            size += enc.Str().size();
        }
        // ProfilerStop();
        auto end = std::chrono::steady_clock::now();
        std::cout << "kun cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms, bytes: " << size << std::endl;
    }

    {
        auto start = std::chrono::steady_clock::now();
        size_t size = 0;
        for (int i = 0; i < n; i++) {
            size += b.SerializeAsString().size();
        }
        auto end = std::chrono::steady_clock::now();
        std::cout << "pb cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms, bytes: " << size << std::endl;
    }
#else
    {
        auto start = std::chrono::steady_clock::now();
        // ProfilerStart("bench.prof");
        size_t size = 0;
        for (int i = 0; i < n; i++) {
            kuntest::AAA aaa;
            kun::Decoder dec(s);
            if (dec.Decode(aaa)) {
                size++;
            }
        }
        // ProfilerStop();
        auto end = std::chrono::steady_clock::now();
        std::cout << "kun cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms, n: " << size << std::endl;
    }

    // std::string s;
    // google::protobuf::json::MessageToJsonString(b, &s);
    // std::cout << s << std::endl;
    {
        auto start = std::chrono::steady_clock::now();
        size_t size = 0;
        for (int i = 0; i < n; i++) {
            pbtest::AAA aaa;
            size += aaa.ParseFromString(s);
        }
        auto end = std::chrono::steady_clock::now();
        std::cout << "pb cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms, n: " << size << std::endl;
    }
#endif

    return 0;
}
