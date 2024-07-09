#include <b.kun.h>
#include <cstdlib>
#include <random>

#include <buffer.h>
#include <gtest/gtest.h>

#include <a.pb.h>

std::mt19937 rng(std::random_device{}());

TEST(Serialize, map)
{
    kuntest::AAA kuna;
    for (int i = 0; i < 1000; ++i) {
        kuna.kvs[rng()] = rng();
    }

    kun::Buffer b;
    kuna.Serialize(b);

    pbtest::AAA pba;
    EXPECT_TRUE(pba.ParseFromArray(b.Str().data(), b.Size()));

    EXPECT_EQ(kuna.kvs.size(), pba.kvs_size());
    for (auto& entry : pba.kvs()) {
        auto iter = kuna.kvs.find(entry.first);
        EXPECT_TRUE(iter != kuna.kvs.end());
        EXPECT_EQ(iter->second, entry.second);
    }
}

TEST(Serialize, output)
{
    pbtest::AAA a;

    a.set_value(123123);
    a.set_d(123.123);
    for (int i = 0; i < 3000; i++) {
        a.add_names(std::to_string(i));
        a.add_ints(i * -12352);
        a.add_floats(i * 311.5);
        a.add_doubles(i * 3231314.7);
        a.mutable_bbb()->add_value(std::to_string(i * 37843));
        a.mutable_bbb()->add_ints(i * 123131);
        a.add_errors(::pbtest::Error(i));
        // a.mutable_kvs()->try_emplace(i, i * 1001);
    }

    kuntest::AAA b;
    b.value = 123123;
    b.d = 123.123;
    b.bbb = kuntest::BBB();
    for (int i = 0; i < 3000; i++) {
        b.names.push_back(std::to_string(i));
        b.ints.push_back(i * -12352);
        b.floats.push_back(i * 311.5);
        b.doubles.push_back(i * 3231314.7);
        b.bbb->value.push_back(std::to_string(i * 37843));
        b.bbb->ints.push_back(i * 123131);
        b.errors.push_back(kuntest::Error(i));
        // b.kvs[i] = i * 1001;
    }
    kun::Buffer buf;
    b.Serialize(buf);

    EXPECT_EQ(a.ByteSizeLong(), b.ByteSize());
    EXPECT_EQ(a.SerializeAsString(), buf.Str());
}