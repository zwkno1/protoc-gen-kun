
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
#include <optional>

#include "util.h"

namespace mytest {

class BBB;

class AAA;

class CCC;

enum Error : int
{
    E1 = 0,
    E2 = 1,
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
            b.WriteString(1, entry);
        }

        if (!ints.empty()) {
            b.WriteIntergerPacked(3, ints);
        }

    }

    inline size_t ByteSize() const
    {
        size_t total_size = 0;
        for (auto& entry : value) {
            uint64_t size = entry.size();
            total_size += ::TagSize(1) + ::LengthDelimitedSize(size);
        }

        if (!ints.empty()) {
            uint64_t size = ::RepeatIntergerByteSize(ints);
            total_size += ::TagSize(3) + ::LengthDelimitedSize(size);
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
    {
    }

    template <typename Buffer>
    inline void Serialize(Buffer& b) const
    {
        for (auto& entry : names) {
            b.WriteString(2, entry);
        }

        if (!kvs.empty()) {
            b.WriteMap(3, kvs);
        }

        if (!kvs2.empty()) {
            b.WriteMap(4, kvs2);
        }

        if (e != 0) {
            b.WriteEnum(5, e);
        }

        if (!date.empty()) {
            b.WriteString(6, date);
        }

        if (value != 0) {
            b.WriteInterger(11, value);
        }

        if(bbb) {
            b.WriteMessage(16, *bbb);
        }

        if (!xx.empty()) {
            b.WriteString(23, xx);
        }

        ::uint32_t tmp_aaa;
        std::memcpy(&tmp_aaa, &aaa, sizeof(aaa));
        if (tmp_aaa != 0) {
            b.WriteFixed(33, tmp_aaa);
        }

        ::uint64_t tmp_d;
        std::memcpy(&tmp_d, &d, sizeof(d));
        if (tmp_d != 0) {
            b.WriteFixed(99, tmp_d);
        }

        if (!ints.empty()) {
            b.WriteIntergerPacked(331, ints);
        }

        if (!uint64s.empty()) {
            b.WriteIntergerPacked(9899, uint64s);
        }

        if (!floats.empty()) {
            b.WriteFixedPacked(12311, floats);
        }

        if (!doubles.empty()) {
            b.WriteFixedPacked(22311, doubles);
        }

        for (auto& entry : bbbs) {
            b.WriteMessage(123123, entry);
        }

        if (!uint32s.empty()) {
            b.WriteIntergerPacked(12312312, uint32s);
        }

        if (!int64s.empty()) {
            b.WriteIntergerPacked(12312364, int64s);
        }

    }

    inline size_t ByteSize() const
    {
        size_t total_size = 0;
        if (value != 0) {
            total_size += ::TagSize(11) + ::IntergerByteSize(value);
        }

        for (auto& entry : names) {
            uint64_t size = entry.size();
            total_size += ::TagSize(2) + ::LengthDelimitedSize(size);
        }

        if (!xx.empty()) {
            uint64_t size = xx.size();
            total_size += ::TagSize(23) + ::LengthDelimitedSize(size);
        }



        if (e != 0) {
            total_size += ::TagSize(5) + ::IntergerByteSize(e);
        }

        if (!date.empty()) {
            uint64_t size = date.size();
            total_size += ::TagSize(6) + ::LengthDelimitedSize(size);
        }

        if(bbb) {
            uint64_t size = bbb->ByteSize();
            total_size += ::TagSize(16) + ::LengthDelimitedSize(size);
        }

        ::uint32_t tmp_aaa;
        std::memcpy(&tmp_aaa, &aaa, sizeof(aaa));
        if (tmp_aaa != 0) {
            total_size += ::TagSize(33) + sizeof(::uint32_t);
        }

        ::uint64_t tmp_d;
        std::memcpy(&tmp_d, &d, sizeof(d));
        if (tmp_d != 0) {
            total_size += ::TagSize(99) + sizeof(::uint64_t);
        }

        for (auto& entry : bbbs) {
            uint64_t size = entry.ByteSize();
            total_size += ::TagSize(123123) + ::LengthDelimitedSize(size);
        }

        if (!ints.empty()) {
            uint64_t size = ::RepeatIntergerByteSize(ints);
            total_size += ::TagSize(331) + ::LengthDelimitedSize(size);
        }

        if (!uint32s.empty()) {
            uint64_t size = ::RepeatIntergerByteSize(uint32s);
            total_size += ::TagSize(12312312) + ::LengthDelimitedSize(size);
        }

        if (!floats.empty()) {
            uint64_t size = ::RepeatedFixedByteSize(floats);
            total_size += ::TagSize(12311) + ::LengthDelimitedSize(size);
        }

        if (!doubles.empty()) {
            uint64_t size = ::RepeatedFixedByteSize(doubles);
            total_size += ::TagSize(22311) + ::LengthDelimitedSize(size);
        }

        if (!int64s.empty()) {
            uint64_t size = ::RepeatIntergerByteSize(int64s);
            total_size += ::TagSize(12312364) + ::LengthDelimitedSize(size);
        }

        if (!uint64s.empty()) {
            uint64_t size = ::RepeatIntergerByteSize(uint64s);
            total_size += ::TagSize(9899) + ::LengthDelimitedSize(size);
        }

        return total_size;
    }

    ::int32_t value;
    std::vector<::std::string> names;
    ::std::string xx;
    std::unordered_map<::int32_t, ::int32_t> kvs;
    std::unordered_map<::int32_t, BBB> kvs2;
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
            b.WriteInterger(1, number);
        }

        if (!str.empty()) {
            b.WriteString(2, str);
        }

    }

    inline size_t ByteSize() const
    {
        size_t total_size = 0;
        if (number != 0) {
            total_size += ::TagSize(1) + ::IntergerByteSize(number);
        }

        if (!str.empty()) {
            uint64_t size = str.size();
            total_size += ::TagSize(2) + ::LengthDelimitedSize(size);
        }

        return total_size;
    }

    ::int32_t number;
    ::std::string str;
};

}  // namespace mytest
