#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "bigint.hpp"

TEST_CASE("BigInt constructors and isZero function") {
    {
        /*
         * Check there are no limbs created when default constructor is used
        */
        BigInt a;
        CHECK(a.isZero());
    }

    {
        /*
         * Check that 0 is treated as no-value in constructor, so no limbs are created
        */
        BigInt a(0);
        CHECK(a.isZero());
    }

    {
        /*
         * Check that single U64 limb constructor works
        */
        BigInt a(57); // 00111001
        CHECK(a.toHex() == "39");
        CHECK_EQ(a.limb.size(), 1);
    }

    {
        /*
            Check that U64 limb constructor cannot receive value higher than U64_max
            and such attempt results in overflow
        */
        BigInt a(UINT64_MAX + 1);
        CHECK_EQ(a.toHex(), "0");
    }
}

TEST_CASE("BigInt fromHex/toHex and normalization function") {
    {
        /*
         * Check that 0's hex string is treated as no limbs
        */
       BigInt a = BigInt::fromHex("0000000000");
       CHECK(a.isZero());
       CHECK_EQ(a.toHex(), "0");
    }

    {
        /*
         * Check that normalization erases all 0's before a non-zero value
        */
       BigInt a = BigInt::fromHex("0000123");
       CHECK(!a.isZero());
       CHECK_EQ(a.toHex(), "123");
       CHECK_EQ(a.limb.size(), 1);
    }

    {
        /*
         * Check that non hex digit destroys constructor and returns empty BigInt
        */
       BigInt a = BigInt::fromHex("11111G111");
       CHECK(a.isZero());
       CHECK_EQ(a.toHex(), "0");
    }

    {
        /*
         * Check that toHex removes leading zeros in MSL
        */
       BigInt a = BigInt::fromHex("002001");
       CHECK(!a.isZero());
       CHECK_EQ(a.limb.size(), 1);
       CHECK_EQ(a.toHex(), "2001");
    }

    {
        /*
         * Check that normalization removes only 0's from limbs == 0
        */
       std::string s = "";
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("1");
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("1");
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("00000000"); // 8 hexes (32 bits)
       s.append("1");

       BigInt a = BigInt::fromHex(s);
       CHECK(!a.isZero());
       CHECK_EQ(a.limb.size(), 3); // Without normalization would be 5

       std::string res = "";
       res.append("100");
       res.append("00000000");
       res.append("00000010");
       res.append("00000000");
       res.append("00000001");
       CHECK_EQ(a.toHex(), res);
    }

    {
        /*
         * Check that 2 limbs of all 0's will result in no limbs in BigInt
        */
       std::string s = "";
       s.append("00000000");
       s.append("00000000");
       s.append("00000000");
       s.append("00000000");
       s.append("00000000");

       BigInt a = BigInt::fromHex(s);
       CHECK(a.isZero());
       CHECK_EQ(a.toHex(), "0");
    }
}

TEST_CASE("BigInt comparison operators") {
    BigInt a = BigInt::fromHex("111");
    BigInt b = BigInt::fromHex("1111");
    BigInt c = BigInt::fromHex("11111");
    BigInt d = BigInt::fromHex("1111F");
    BigInt e = BigInt::fromHex("FFF");
    BigInt f = BigInt::fromHex("111");
    BigInt g = BigInt::fromHex("1111111111111111111111111111111111111111111111111"); // 49 nibbles
    BigInt h = BigInt::fromHex("1111111111111111111111111111111111111111111111111");
    BigInt i = BigInt::fromHex("2111111111111111111111111111111111111111111111111");
    
    CHECK_LT(a, b);
    CHECK_LT(c, d);
    CHECK_EQ(a, f);
    CHECK_LE(a, b);
    CHECK_LE(a, f);
    CHECK_NE(a, e);
    CHECK_NE(c, d);
    CHECK_GT(d, c);
    CHECK_GT(d, a);
    CHECK_GE(a, f);
    CHECK_GE(d, c);
    CHECK_EQ(g, h);
    CHECK_LE(g, h);
    CHECK_GE(g, h);
    CHECK_GE(i, h);
    CHECK_GT(i, h);
}

TEST_CASE("BigInt addition") {
    {
        /*
         * Check if long consecutive carry is working
        */
        BigInt a = BigInt::fromHex("FFFFFFFFFFFFFFFFF"); // 17 nibbles
        BigInt b = BigInt::fromHex("FFFFFFFFFFFFFFFFF");
        a += b;

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toHex(), "1FFFFFFFFFFFFFFFFE");
    }

    {
        /*
         * Check if carry returned from MSB in U64 creates a new limb
        */
        BigInt a = BigInt::fromHex("8000000000000000"); // 16 nibbles
        BigInt b = BigInt::fromHex("8000000000000000");
        a += b;

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toHex(), "10000000000000000"); // 17 nibbles
    }

    {
        /*
         * Check if large values work
        */
        std::string s = "";
        s.append("00F00F00");
        s.append("00F00F00");
        s.append("00F00F00");
        s.append("00F00F00");
        s.append("00F00F00");
        s.append("00F00F00");
        s.append("00F00F00");

        BigInt a = BigInt::fromHex(s);
        BigInt b = BigInt::fromHex(s);
        BigInt c = a + b;

        std::string res = "";
        res.append("1E01E00");
        res.append("01E01E00");
        res.append("01E01E00");
        res.append("01E01E00");
        res.append("01E01E00");
        res.append("01E01E00");
        res.append("01E01E00");

        CHECK(!c.isZero());
        CHECK_EQ(c.limb.size(), 4);
        CHECK_EQ(c.toHex(), res);
    }
}

TEST_CASE("BigInt substraction") {
    {
        /*
         * Check that we catch an exception when minuend is smaller than substrahend
        */
        std::string s1 = "";
        s1.append("0000EFFF");
        s1.append("00000000");
        s1.append("00000000");
        s1.append("00000000");
        s1.append("00000000");

        std::string s2 = "";
        s2.append("0000FFFF");
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000000");

        BigInt a = BigInt::fromHex(s1);
        BigInt b = BigInt::fromHex(s2);

        CHECK_THROWS_WITH_MESSAGE(a -= b, "BigInt::substract: result is negative", "std::runtime_error");
    }

    {
        /*
         * Check that borrow mechanism works correctly through a long consecutive action
        */
        std::string s1 = "";
        s1.append("F0000000");
        s1.append("00000000");
        s1.append("00000000");
        s1.append("00000000");
        s1.append("00000000");
        s1.append("00000000");
        s1.append("00000000");

        std::string s2 = "";
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000000");
        s2.append("00000001");

        BigInt a = BigInt::fromHex(s1);
        BigInt b = BigInt::fromHex(s2);

        a -= b;

        std::string res = "";
        res.append("EFFFFFFF");
        res.append("FFFFFFFF");
        res.append("FFFFFFFF");
        res.append("FFFFFFFF");
        res.append("FFFFFFFF");
        res.append("FFFFFFFF");
        res.append("FFFFFFFF");

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 4);
        CHECK_EQ(a.toHex(), res);
    }

    {
        /*
         * Check that more complicated example works correctly
        */
        std::string s1 = "";
        s1.append("FEFEFEFE");
        s1.append("FEFEFEFE");
        s1.append("FEFEFEFE");

        std::string s2 = "";
        s2.append("EFEFEFEF");
        s2.append("EFEFEFEF");
        s2.append("EFEFEFEF");

        BigInt a = BigInt::fromHex(s1);
        BigInt b = BigInt::fromHex(s2);

        a -= b;

        std::string res = "";
        res.append("F0F0F0F");
        res.append("0F0F0F0F");
        res.append("0F0F0F0F");

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toHex(), res);
    }

    {
        /*
         * Check that a long consecutive action of normalization works correctly
        */
        std::string s1 = "";
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");

        BigInt a = BigInt::fromHex(s1);
        BigInt b = BigInt::fromHex(s1);

        a -= b;

        CHECK(a.isZero());
        CHECK_EQ(a.toHex(), "0");
    }
}

TEST_CASE("BigInt multiplication") {
    {
        /*
         * Simple multiplication that is easy to predict
         *
         * s = 0x3F * (2^64 + 2^32 + 1) [t = 2^32]
         * s = 63 * (t^2 + t + 1)
         * res = s^2 = 3969 * (t^2 + t + 1)^2
         * res = 0xF81 * (t^4 + 2t^3 + 3t^2 + 2t + 1)
         * 
         * This means that res has:
         *  1) 0xF81 on bit 1
         *  2) 0xF81 * 0x2 = 0x1F02 on bit 32
         *  3) 0xF81 * 0x3 = 0x2E83 on bit 64
         *  4) 0xF81 * 0x2 = 0x1F02 on bit 96
         *  5) 0xF81 on bit 256
        */
        std::string s = ""; // 0xF81 * (2^64 + 2^32 + 1)
        s.append("0000003F");
        s.append("0000003F");
        s.append("0000003F");

        BigInt a = BigInt::fromHex(s);
        BigInt b = BigInt::fromHex(s);
        a *= b;

        std::string res = "";
        res.append("F81");
        res.append("00001F02");
        res.append("00002E83");
        res.append("00001F02");
        res.append("00000F81");

        CHECK(!a.isZero());
        CHECK(!b.isZero());
        CHECK_EQ(a.limb.size(), 3);
        CHECK_EQ(a.toHex(), res);
    }

    {
        /*
         * Check that new limb is created when result does not fit in the same size
         *
         * s = 0xF * 2^59
         * s^2 = 0xF * 0xF * 2^118 = 0xE1 * 2^118
        */
       std::string s = "";
       s.append("F0000000");
       s.append("00000000");

       BigInt a = BigInt::fromHex(s);
       BigInt b = BigInt::fromHex(s);

       a *= b;

       std::string res = "";
       res.append("E1000000");
       res.append("00000000");
       res.append("00000000");
       res.append("00000000");

       CHECK(!a.isZero());
       CHECK_EQ(a.limb.size(), 2);
       CHECK_EQ(a.toHex(), res);
    }
}
