#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "buffer.h"
#include "test/a.kun.h"

int main(int argc, char* argv[])
{
    mytest::AAA aaa;
    aaa.value = 123123;
    aaa.d = 123.123;
    aaa.bbb = mytest::BBB();
    for (int i = 0; i < 3000000; i++) {
        aaa.names.push_back(std::to_string(i));
        aaa.ints.push_back(i * -12352);
        aaa.floats.push_back(i * 311.5);
        aaa.doubles.push_back(i * 3231314.7);
        aaa.bbb->value.push_back(std::to_string(i * 37843));
        aaa.bbb->ints.push_back(i * 123131);
        aaa.errors.push_back(mytest::Error(i));
        //  aaa.kvs[i] = i * 1001;
        //  std::cout << aaa.ByteSize() << std::endl;
    }

    kun::Buffer b;
    aaa.Serialize(b);
    std::ofstream f("./kun.bin", std::fstream::binary);
    f.write((const char*)b.Data(), b.Size());
}