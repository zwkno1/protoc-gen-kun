
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
    std::string type = "encode";

    if (argc < 2) {
        return -1;
    }

    if (argc >= 3) {
        type = argv[1];
        if (type != "encode" && type != "decode") {
            return -1;
        }
    }

    n = atoi(argv[argc - 1]);

    std::cout << "benchmark " << type << " n: " << n << std::endl;

    kuntest::AAA a = GenAAA();
    pbtest::AAA b = ToPb(a);

    std::string s = b.SerializeAsString();

    if (type == "encode") {
        {
            auto start = std::chrono::steady_clock::now();
            // ProfilerStart("kun.prof");
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
            // ProfilerStart("pb.prof");
            size_t size = 0;
            for (int i = 0; i < n; i++) {
                size += b.SerializeAsString().size();
            }
            // ProfilerStop();
            auto end = std::chrono::steady_clock::now();
            std::cout << "pb cost: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                      << "ms, bytes: " << size << std::endl;
        }
    } else {
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
    }

    return 0;
}
