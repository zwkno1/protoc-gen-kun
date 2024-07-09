
/***************************************************************************************************
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::...::::::::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::::..::::....::::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::::^!?YYJ?7!^..::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::^7Y5PPPPPP5Y!:::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::........^YPGGGGGPPGGP!::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::...:^!JYY^^JGBBGBBG?JPGP!::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::.:!YPGBBBBP!~PBBBBGB57??!:::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::..^YBBBBBBBBBB?^5BBBBBBP7:.:::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::.:!P#BBBBBBBBBBB?^P#BBBGGY^.:::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::..~5B#B###BBBBBBBBB~!GY7~!7J7:.::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::.:?B###&#Y5BBBBBBB##Y!!~?PB#BBG7::::::::::::::::::::::::::::::::::::
^::::::::::::::::::::::::::::::.^5####&#J:?#BBBB####575B#######Y:.::::::::::::::::::::::::::::::::::
^^^::..:::::::::::::::::::::::.:Y&###&&J.!G#BB#####B7~B########5:.:.::::::::::::::::::::::::::::::::
^^^~^^::.::::::::::::::::::::::.!P#&###?7B#BB######G~J########&#?::.::::::::::::::::::::::::::::::::
^^~~~~^^::.:::::::::::::::::::::.:!P###BB#BBB######Y^P#########&G^.:::::::::::::::::::::::::::::::::
^~~~~~~~~^:::::::::::::::::::::::..:?G###BBB#######7~B#########&G^.:::::::::::::::::::::::::::::::::
~~~~~~~~~~~^^:::.::::::::::::::::::..:?PB#########B7J###########5:::::::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~^:::.::::::::::::::::::..^7555PGBBB#GYG##########B~.::::::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~~~^^::::::::::::::::::::7J?JJJJJJYYYYJYY5PGBB#&&&Y::::::::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~~~~~^^:::.::::::::::::.~YJ?JJJJJJJYYYYYYJYYY55??!:::::::::::::::::::::::::::::::::::::
^~~~~~~~~~~~~~~~~~~~~^:::::::::::::.:7J??JJJJJJJYYYYYJYYYYYJ^..:::::::::::::::::::::::::::::::::::::
:^^~~~~~~~~~~~~~~~~~~~~^^::::::::::~7??JJJJJYYYYYYYYYYYYYYYY7^..::::::::::::::::::::::::::::::::::::
::::^~~~~~~~~~~~~~~~~~~~~^^:::::::!????JJJJYYYYYY55YYYYYYYYJJ?!:::::::::::::::::::::::::::::::::::::
::::::^~~~~~~~~~~~~~~~~~~~~~^^::~7??JJJJYYYY5555555555555YYJJJJ7::::::::::::::::::::::::::::::::::::
:::::::::^~~~~~~~~~~~~~~~~~~~~~!?JJJJJJYYY555J?!^^!?Y55555YYYJJ?^:::::::::::::::::::::::::::::::::::
:::::::::::^~~~~~~~~~~~~~~~~~~7?JJJJYYY555J7^:......^!Y555YYYYJJ?^::::::::::::::::::::::::::::::::::
:::::::::::::^^~~~~~~~~~~~~~~7?JJJJJY55Y7~:..::::::::.^?55YYYYYJJ7^:::::::::::::::::::::::::::::::::
:::::::::::::::^^~~~~~~~~~~~7JJYYYY55Y?^:.::::::::::::.:!Y5555YYYY7:::::::::::::::::::::::::::::::::
::::::::::::::::::^~~~~~~~~~7??J5PP5J!~~^^::::::::::::::.:!YP555YYJ!::::::::::::::::::::::::::::::::
::::::::::::::::::::^^~~~~~77?JJY5J!~~~~~~~^::::::::::::::.^7Y555YJ?7:::::::::::::::::::::::::::::::
:::::::::::::::::::::::^~~~7???JYJ!~~~~~~~~~~^::::::::::::::.:7YYJ???!::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~~~~~~~~~~~~~!7???JY?!!!!!!~~~~~~~^^^^^^^^^^^^^^^^7JJ????!^^^^~~^^^^~~^^^^^^^^^^^^^^^^^
~~~~~~~~~~~~~~~~~~~~~~~~~~!???JJY7~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?JJ???7~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~!???JYJ!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!JJJ???!~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~!??JJJ7~~~~~~~^^^^^^^^^^^^^^^^~~~~^^^^^7JJ???7~^^^^~~~~~~~~~~~~~~~~~~~~~~~
~~~~~^^^^~~~~~~~~~~~~~~~~~7???JJ~~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^~7JJ???~^^^^~^^^^^~~~~~~~~~~~~~~~~~
***************************************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <optional>

#include "kun.h"

namespace mytest {

class BBB;

class AAA;

class CCC;

enum Error : int32_t
{
    E1 = 0,
    E2 = 2,
    E3 = -1,
};

class BBB 
{
public:
    BBB()
      : value()
      , ints()
    {
    }

    template <typename Buffer>
    inline void Serialize(Buffer& b) const
    {
        for (auto& entry : value) {
            b.template Write<1>(entry);
        }

        if (!ints.empty()) {
            b.template Write<3>(ints);
        }

    }

    inline size_t ByteSize() const
    {
        size_t total_size = 0;
        for (auto& entry : value) {
            total_size += ::kun::TaggedByteSize<1>(entry);
        }

        if (!ints.empty()) {
            total_size += ::kun::TaggedByteSize<3>(ints);
        }

        return total_size;
    }

    std::vector<::std::string> value;
    std::vector<::int32_t> ints;
};

class AAA 
{
public:
    AAA()
      : value(0)
      , names()
      , xx()
      , kvs()
      , kvs2()
      , e()
      , date()
      , bbb()
      , aaa(0)
      , d(0)
      , bbbs()
      , ints()
      , uint32s()
      , floats()
      , doubles()
      , int64s()
      , uint64s()
      , errors()
    {
    }

    template <typename Buffer>
    inline void Serialize(Buffer& b) const
    {
        for (auto& entry : names) {
            b.template Write<2>(entry);
        }

        for(auto & entry : kvs) {
            b.template Write<3>(entry);
        }

        for(auto & entry : kvs2) {
            b.template Write<4>(entry);
        }

        if (e != 0) {
            b.template Write<5>(static_cast<uint64_t>(e));
        }

        if (!date.empty()) {
            b.template Write<6>(date);
        }

        if (value != 0) {
            b.template Write<11>(value);
        }

        if(bbb) {
            b.template Write<16>(*bbb);
        }

        if (!xx.empty()) {
            b.template Write<23>(xx);
        }

        ::uint32_t tmp_aaa = std::bit_cast<::uint32_t>(aaa);
        if (tmp_aaa != 0) {
            b.template Write<33>(aaa);
        }

        ::uint64_t tmp_d = std::bit_cast<::uint64_t>(d);
        if (tmp_d != 0) {
            b.template Write<99>(d);
        }

        if (!ints.empty()) {
            b.template Write<331>(ints);
        }

        if (!uint64s.empty()) {
            b.template Write<9899>(uint64s);
        }

        if (!floats.empty()) {
            b.template Write<12311>(floats);
        }

        if (!errors.empty()) {
            b.template Write<12312>(errors);
        }

        if (!doubles.empty()) {
            b.template Write<22311>(doubles);
        }

        for (auto& entry : bbbs) {
            b.template Write<123123>(bbbs);
        }

        if (!uint32s.empty()) {
            b.template Write<12312312>(uint32s);
        }

        if (!int64s.empty()) {
            b.template Write<12312364>(int64s);
        }

    }

    inline size_t ByteSize() const
    {
        size_t total_size = 0;
        if (value != 0) {
            total_size += ::kun::TaggedByteSize<11>(value);
        }

        for (auto& entry : names) {
            total_size += ::kun::TaggedByteSize<2>(entry);
        }

        if (!xx.empty()) {
            total_size += ::kun::TaggedByteSize<23>(xx);
        }

        for(auto & entry : kvs) {
            total_size += ::kun::TaggedByteSize<3>(entry);
        }

        for(auto & entry : kvs2) {
            total_size += ::kun::TaggedByteSize<4>(entry);
        }

        if (e != 0) {
            total_size += ::kun::TaggedByteSize<5>(e);
        }

        if (!date.empty()) {
            total_size += ::kun::TaggedByteSize<6>(date);
        }

        if(bbb) {
            auto size = ::kun::ByteSize(*bbb);
            if(size != 0) {
                total_size += ::kun::TagSize(16) + ::kun::LengthDelimitedSize(size);
            }
        }

        ::uint32_t tmp_aaa = std::bit_cast<::uint32_t>(aaa);
        if (tmp_aaa != 0) {
            total_size += ::kun::TaggedByteSize<33>(aaa);
        }

        ::uint64_t tmp_d = std::bit_cast<::uint64_t>(d);
        if (tmp_d != 0) {
            total_size += ::kun::TaggedByteSize<99>(d);
        }

        for (auto& entry : bbbs) {
            total_size += ::kun::TaggedByteSize<123123>(entry);
        }

        if (!ints.empty()) {
            total_size += ::kun::TaggedByteSize<331>(ints);
        }

        if (!uint32s.empty()) {
            total_size += ::kun::TaggedByteSize<12312312>(uint32s);
        }

        if (!floats.empty()) {
            total_size += ::kun::TaggedByteSize<12311>(floats);
        }

        if (!doubles.empty()) {
            total_size += ::kun::TaggedByteSize<22311>(doubles);
        }

        if (!int64s.empty()) {
            total_size += ::kun::TaggedByteSize<12312364>(int64s);
        }

        if (!uint64s.empty()) {
            total_size += ::kun::TaggedByteSize<9899>(uint64s);
        }

        if (!errors.empty()) {
            total_size += ::kun::TaggedByteSize<12312>(errors);
        }

        return total_size;
    }

    ::int32_t value;
    std::vector<::std::string> names;
    ::std::string xx;
    std::map<::int32_t, ::int32_t> kvs;
    std::map<::int32_t, BBB> kvs2;
    Error e;
    ::std::string date;
    std::optional<BBB> bbb;
    float aaa;
    double d;
    std::vector<BBB> bbbs;
    std::vector<::int32_t> ints;
    std::vector<::uint32_t> uint32s;
    std::vector<float> floats;
    std::vector<double> doubles;
    std::vector<::int64_t> int64s;
    std::vector<::uint64_t> uint64s;
    std::vector<Error> errors;
};

class CCC 
{
public:
    CCC()
      : number(0)
      , str()
    {
    }

    template <typename Buffer>
    inline void Serialize(Buffer& b) const
    {
        if (number != 0) {
            b.template Write<1>(number);
        }

        if (!str.empty()) {
            b.template Write<2>(str);
        }

    }

    inline size_t ByteSize() const
    {
        size_t total_size = 0;
        if (number != 0) {
            total_size += ::kun::TaggedByteSize<1>(number);
        }

        if (!str.empty()) {
            total_size += ::kun::TaggedByteSize<2>(str);
        }

        return total_size;
    }

    ::int32_t number;
    ::std::string str;
};

}  // namespace mytest
namespace kun {
template<>
struct is_enum<::mytest::Error> : public std::true_type
{
};

template<>
struct is_message<::mytest::BBB> : public std::true_type
{
};

template<>
struct is_message<::mytest::AAA> : public std::true_type
{
};

template<>
struct is_message<::mytest::CCC> : public std::true_type
{
};

}  // namespace kun
