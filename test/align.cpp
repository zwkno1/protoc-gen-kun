#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

enum class A : int32_t
{
    A1 = 0,
};

struct B
{
    uint32_t a;
    uint64_t b;
    double c;
    float d;
};

int main(int argc, char* argv[])
{
    // clang-format off
    std::cout << "bool   " << sizeof(bool)                                  << ", " << alignof(bool)                                   << std::endl;
    std::cout << "u32    " << sizeof(uint32_t)                              << ", " << alignof(uint32_t)                               << std::endl;
    std::cout << "u64    " << sizeof(uint64_t)                              << ", " << alignof(uint64_t)                               << std::endl;
    std::cout << "enum   " << sizeof(A)                                     << ", " << alignof(A)                                      << std::endl;
    std::cout << "float  " << sizeof(float)                                 << ", " << alignof(float)                                  << std::endl;
    std::cout << "double " << sizeof(double)                                << ", " << alignof(double)                                 << std::endl;
    std::cout << "string " << sizeof(std::string)                           << ", " << alignof(std::string)                            << std::endl;
    std::cout << "vector " << sizeof(std::vector<int32_t>)                  << ", " << alignof(std::vector<int32_t>)                   << std::endl;
    std::cout << "map    " << sizeof(std::unordered_map<int32_t, int32_t>)  << ", " << alignof(std::unordered_map<int32_t, int32_t>)   << std::endl;
    std::cout << "B      " << sizeof(std::unique_ptr<B>)                    << ", " << alignof(std::unique_ptr<B>)                     << std::endl;

    return 0;
}