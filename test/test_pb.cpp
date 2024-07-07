#include <fstream>
#include <string>

#include "test/a.pb.h"

int main(int argc, char* argv[])
{
    mytest::AAA aaa;
    aaa.set_value(123123);
    aaa.set_d(123.123);
    for (int i = 0; i < 3; i++) {
        aaa.add_names(std::to_string(i));
        aaa.add_ints(i);
        aaa.add_floats(i * 3.5);
        aaa.add_doubles(i * 4.7);
        aaa.mutable_bbb()->add_value(std::to_string(i * 10));
        aaa.mutable_bbb()->add_ints(i);
    }
    auto s = aaa.SerializeAsString();
    std::ofstream f("./pb.bin", std::fstream::binary);
    f.write((const char*)s.data(), s.size());
}