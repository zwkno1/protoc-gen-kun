#include <fstream>
#include <string>

#include "test/a.pb.h"

int main(int argc, char* argv[])
{
    mytest::AAA aaa;
    aaa.set_value(123123);
    aaa.set_d(123.123);
    for (int i = 0; i < 3000000; i++) {
        aaa.add_names(std::to_string(i));
        aaa.add_ints(i * -12352);
        aaa.add_floats(i * 311.5);
        aaa.add_doubles(i * 3231314.7);
        aaa.mutable_bbb()->add_value(std::to_string(i * 37843));
        aaa.mutable_bbb()->add_ints(i * 123131);
        aaa.add_errors(::mytest::Error(i));
        // aaa.mutable_kvs()->try_emplace(i, i * 1001);
        //   std::cout << aaa.ByteSizeLong() << std::endl;
    }
    auto s = aaa.SerializeAsString();
    std::ofstream f("./pb.bin", std::fstream::binary);
    f.write((const char*)s.data(), s.size());
}