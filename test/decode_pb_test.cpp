#include "a.kun.h"
#include "b.pb.h"
#include "codec.h"
#include "test/helper.h"

void CheckDecode(const pbtest::AAA& b)
{
    auto data = b.SerializeAsString();

    kuntest::AAA a;
    kun::Decoder dec(data);
    EXPECT_TRUE(dec.Decode(a));

    EXPECT_EQ(b.ByteSizeLong(), a.ByteSize());
    ExpectEQ(a, b);
}

TEST(Decode, numeric)
{
    kuntest::AAA a;
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

    CheckDecode(ToPb(a));
}

TEST(DecodePb, map)
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

    CheckDecode(ToPb(a));
}
