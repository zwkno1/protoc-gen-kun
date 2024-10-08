#include <bit>
#include <string>
#include <type_traits>

#include <codec.h>
#include <kun.h>

#include "a.kun.h"
#include "b.pb.h"
#include "test/helper.h"

void CheckEncode(const kuntest::AAA& a)
{
    kun::Encoder enc;
    enc.Encode(a);

    pbtest::AAA b;
    EXPECT_TRUE(b.ParseFromString(enc.Str()));

    EXPECT_EQ(a.ByteSize(), b.ByteSizeLong());
    ExpectEQ(a, b);
}

TEST(EncodePb, primitive)
{
    kuntest::AAA a;
    a.b = true;
    a.u32 = 2;

    GenRand(a.i32);
    GenRand(a.u32);
    GenRand(a.i64);
    GenRand(a.u64);
    GenRand(a.e);
    GenRand(a.f);
    GenRand(a.d);
    GenRand(a.s);
    GenRand(a.bt);
    GenRand(a.b);

    GenRand(a.s32);
    GenRand(a.s64);
    GenRand(a.sf32);
    GenRand(a.sf64);
    GenRand(a.f32);
    GenRand(a.f64);

    GenRandRepeated(a.i32s);
    GenRandRepeated(a.u32s);
    GenRandRepeated(a.i64s);
    GenRandRepeated(a.u64s);
    GenRandRepeated(a.es);
    GenRandRepeated(a.fs);

    GenRandRepeated(a.ds);
    GenRandRepeated(a.ss);
    GenRandRepeated(a.bts);
    GenRandRepeated(a.bs);

    GenRandRepeated(a.s32s);
    GenRandRepeated(a.s64s);
    GenRandRepeated(a.sf32s);
    GenRandRepeated(a.sf64s);
    GenRandRepeated(a.f32s);
    GenRandRepeated(a.f64s);

    // pbtest::AAA b = ToPb(a);
    // Print(b.SerializeAsString());
    // kun::Encoder enc;
    // enc.Encode(a);
    // Print(enc.Str());
    // std::cout << a.ByteSize() << ", " << b.ByteSizeLong() << std::endl;

    CheckEncode(a);
}

TEST(EncodePb, map)
{
    kuntest::AAA a;

    for (int i = 0; i < rng() % 100; ++i) {
        a.kvs[rng()] = rng();
    }

    a.kvs2[""] = kuntest::BBB{};

    for (int i = 0; i < 10; ++i) {
        std::string key;
        GenRand(key);
        kuntest::BBB bbb;
        GenRandRepeated(bbb.ints);
        GenRandRepeated(bbb.value);
        a.kvs2.emplace(std::move(key), std::move(bbb));
    }

    CheckEncode(a);
}

TEST(EncodePb, misc)
{
    kuntest::AAA a = GenAAA();

    CheckEncode(a);
}
