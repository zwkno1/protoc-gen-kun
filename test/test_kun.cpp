#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "buffer.h"
#include "test/a.kun.h"

int main(int argc, char* argv[])
{
    double a = 0.f;
    mytest::AAA aaa;
    aaa.value = 123123;
    aaa.d = 123.123;
    aaa.bbb = mytest::BBB();
    for (int i = 0; i < 3; i++) {
        aaa.names.push_back(std::to_string(i));
        aaa.ints.push_back(i);
        aaa.floats.push_back(i * 3.5);
        aaa.doubles.push_back(i * 4.7);
        aaa.bbb->value.push_back(std::to_string(i * 10));
        aaa.bbb->ints.push_back(i);
    }

    Buffer b;
    aaa.Serialize(b);
    std::ofstream f("./my.bin", std::fstream::binary);
    f.write((const char*)b.Data(), b.Size());
}