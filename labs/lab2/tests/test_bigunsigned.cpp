#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "bigunsigned.hpp"

TEST_CASE("BigUnsigned constructors, isZero, isOne") {
    {
        /*
         * Check there are no limbs created when default constructor is used
        */
        BigUnsigned a;
        CHECK(a.isZero());
    }

    {
        /*
         * Check that 0 is treated as no-value in constructor, so no limbs are created
        */
        BigUnsigned a(0);
        CHECK(a.isZero());
    }

    {
        /*
         * Check that single U64 limb constructor works
        */
        BigUnsigned a(57); // 00111001
        CHECK(a.toBase16() == "39");
        CHECK_EQ(a.limb.size(), 1);
    }

    {
        /*
         * Check that U64 limb constructor cannot receive value higher than U64_max
         * and such attempt results in overflow
        */
        BigUnsigned a(UINT64_MAX + 1);
        CHECK_EQ(a.toBase16(), "0");
    }

    {
        BigUnsigned a(1);
        BigUnsigned b(0);

        CHECK(a.isOne());
        CHECK(!b.isOne());
        CHECK(!a.isZero());
        CHECK(b.isZero());
        CHECK_EQ(a.limb.size(), 1);
        CHECK_EQ(b.limb.size(), 0);
    }

    {
        /*
         * Check that copy constructor works
        */
        BigUnsigned a(32);
        BigUnsigned b(a);

        CHECK_EQ(a, b);

        b += 10;

        CHECK_NE(a, b);
    }

    {
        /*
         * Check that assignment operator works for uint64_t
        */
        const uint64_t val = 1234ULL;

        BigUnsigned a;
        a = val;

        CHECK_EQ(a, val);
    }
}

TEST_CASE("BigUnsigned fromBase16/toBase16 and normalization function") {
    {
        /*
         * Check that 0's hex string is treated as no limbs
        */
        BigUnsigned a = BigUnsigned::fromBase16("0000000000");
        CHECK(a.isZero());
        CHECK_EQ(a.toBase16(), "0");
    }

    {
        /*
         * Check that normalization erases all 0's before a non-zero value
        */
        BigUnsigned a = BigUnsigned::fromBase16("0000123");
        CHECK(!a.isZero());
        CHECK_EQ(a.toBase16(), "123");
        CHECK_EQ(a.limb.size(), 1);
    }

    {
        /*
         * Check that non hex digit destroys constructor and returns empty BigUnsigned
        */
        CHECK_THROWS_WITH_MESSAGE(BigUnsigned a = BigUnsigned::fromBase16("11111G111"), "BigUnsigned::fromBase16 invalid character.", "std::runtime_error");
    }

    {
        /*
         * Check that toBase16 removes leading zeros in MSL
        */
        BigUnsigned a = BigUnsigned::fromBase16("002001");
        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 1);
        CHECK_EQ(a.toBase16(), "2001");
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

        BigUnsigned a = BigUnsigned::fromBase16(s);
        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 3); // Without normalization would be 5

        std::string res = "";
        res.append("100");
        res.append("00000000");
        res.append("00000010");
        res.append("00000000");
        res.append("00000001");
        CHECK_EQ(a.toBase16(), res);
    }

    {
        /*
         * Check that 2 limbs of all 0's will result in no limbs in BigUnsigned
        */
        std::string s = "";
        s.append("00000000");
        s.append("00000000");
        s.append("00000000");
        s.append("00000000");
        s.append("00000000");

        BigUnsigned a = BigUnsigned::fromBase16(s);
        CHECK(a.isZero());
        CHECK_EQ(a.toBase16(), "0");
    }
}

TEST_CASE("BigUnsigned comparison operators") {
    BigUnsigned a = BigUnsigned::fromBase16("111");
    BigUnsigned b = BigUnsigned::fromBase16("1111");
    BigUnsigned c = BigUnsigned::fromBase16("11111");
    BigUnsigned d = BigUnsigned::fromBase16("1111F");
    BigUnsigned e = BigUnsigned::fromBase16("FFF");
    BigUnsigned f = BigUnsigned::fromBase16("111");
    BigUnsigned g = BigUnsigned::fromBase16("1111111111111111111111111111111111111111111111111"); // 49 nibbles
    BigUnsigned h = BigUnsigned::fromBase16("1111111111111111111111111111111111111111111111111");
    BigUnsigned i = BigUnsigned::fromBase16("2111111111111111111111111111111111111111111111111");
    
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

TEST_CASE("BigUnsigned addition") {
    {
        /*
         * Check if long consecutive carry is working
        */
        BigUnsigned a = BigUnsigned::fromBase16("FFFFFFFFFFFFFFFFF"); // 17 nibbles
        BigUnsigned b = BigUnsigned::fromBase16("FFFFFFFFFFFFFFFFF");
        a += b;

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toBase16(), "1FFFFFFFFFFFFFFFFE");
    }

    {
        /*
         * Check if carry returned from MSB in U64 creates a new limb
        */
        BigUnsigned a = BigUnsigned::fromBase16("8000000000000000"); // 16 nibbles
        BigUnsigned b = BigUnsigned::fromBase16("8000000000000000");
        a += b;

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toBase16(), "10000000000000000"); // 17 nibbles
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

        BigUnsigned a = BigUnsigned::fromBase16(s);
        BigUnsigned b = BigUnsigned::fromBase16(s);
        BigUnsigned c = a + b;

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
        CHECK_EQ(c.toBase16(), res);
    }
}

TEST_CASE("BigUnsigned substraction") {
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

        BigUnsigned a = BigUnsigned::fromBase16(s1);
        BigUnsigned b = BigUnsigned::fromBase16(s2);

        CHECK_THROWS_WITH_MESSAGE(a -= b, "BigUnsigned::substract: result is negative", "std::runtime_error");
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

        BigUnsigned a = BigUnsigned::fromBase16(s1);
        BigUnsigned b = BigUnsigned::fromBase16(s2);

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
        CHECK_EQ(a.toBase16(), res);
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

        BigUnsigned a = BigUnsigned::fromBase16(s1);
        BigUnsigned b = BigUnsigned::fromBase16(s2);

        a -= b;

        std::string res = "";
        res.append("F0F0F0F");
        res.append("0F0F0F0F");
        res.append("0F0F0F0F");

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toBase16(), res);
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

        BigUnsigned a = BigUnsigned::fromBase16(s1);
        BigUnsigned b = BigUnsigned::fromBase16(s1);

        a -= b;

        CHECK(a.isZero());
        CHECK_EQ(a.toBase16(), "0");
    }
}

TEST_CASE("BigUnsigned multiplication") {
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

        BigUnsigned a = BigUnsigned::fromBase16(s);
        BigUnsigned b = BigUnsigned::fromBase16(s);
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
        CHECK_EQ(a.toBase16(), res);
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

        BigUnsigned a = BigUnsigned::fromBase16(s);
        BigUnsigned b = BigUnsigned::fromBase16(s);

        a *= b;

        std::string res = "";
        res.append("E1000000");
        res.append("00000000");
        res.append("00000000");
        res.append("00000000");

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toBase16(), res);
    }
}

TEST_CASE("BigUnsigned division and modulo") {
    {
        /*
         * Check different use cases of divmod
        */
        BigUnsigned a(1);
        BigUnsigned b(0);

        CHECK_THROWS_WITH_MESSAGE(a /= b, "BigUnsigned::divmod division by zero.", "std::runtime_error");

        b / a;
        CHECK_EQ(a, 1ULL);
        CHECK_EQ(b, 0ULL);

        a /= a;
        CHECK_EQ(a, 1ULL);

        CHECK_THROWS_WITH_MESSAGE(b /= b, "BigUnsigned::divmod division by zero.", "std::runtime_error");

        b /= a;
        CHECK_EQ(b, 0);

        a = 1223;
        b = 2222;

        a /= b;
        CHECK_EQ(a, 0);
    }

    {
        /*
         * Check that a larger example for division works correctly
        */
        std::string s = "";
        s.append("1040104");
        s.append("01040104");
        s.append("01040104");
        s.append("01040104");
        s.append("01040104");

        std::string s2 = "";
        s2.append("104010");
        s2.append("00104010");
        s2.append("00104010");

        BigUnsigned a = BigUnsigned::fromBase16(s);
        BigUnsigned b = BigUnsigned::fromBase16(s2);

        CHECK_EQ(b.limb.size(), 2);

        BigUnsigned c = a / b;

        std::string res = "";
        res.append("10");
        res.append("00003F03");
        res.append("B2337FCD");

        CHECK_EQ(c.toBase16(), res);

        BigUnsigned d = a % b;
        BigUnsigned e = (b * c) + d;

        CHECK_EQ(e.toBase16(), s);
    }

    {
        /*
         * Check that a larger example for division works correctly with totally random numbers
        */
        std::string s = "";
        s.append("53515152");
        s.append("64236252");
        s.append("75647454");
        s.append("AFFAACDA");
        s.append("11111111");

        std::string s2 = "";
        s2.append("55353FFF");
        s2.append("30303032");
        s2.append("00000001");

        BigUnsigned a = BigUnsigned::fromBase16(s);
        BigUnsigned b = BigUnsigned::fromBase16(s2);

        CHECK_EQ(b.limb.size(), 2);

        BigUnsigned c = a / b;

        std::string res = "";
        res.append("FA521154");
        res.append("9210C077");

        CHECK_EQ(c.toBase16(), res);

        BigUnsigned d = a % b;
        BigUnsigned e = (b * c) + d;

        CHECK_EQ(e.toBase16(), s);
    }

    {
        /*
         * Check that modulo works for a larger example
        */
        std::string s1 = "1000000000000000000";
        std::string s2 = "100000000";

        BigUnsigned v1 = BigUnsigned::fromBase16(s1);
        BigUnsigned v2 = BigUnsigned::fromBase16(s2);
        BigUnsigned v3 = v1 % v2;
        BigUnsigned v4 = v2 % v1;

        CHECK_EQ(v3, 0ULL);
        CHECK_EQ(v4, v2);

        s1 = "1000000000001111011";
        s2 = "100000000";

        v1 = BigUnsigned::fromBase16(s1);
        v2 = BigUnsigned::fromBase16(s2);
        v3 = v1 % v2;

        CHECK_EQ(v3.toBase16(), "1111011");
    }
}

TEST_CASE("BigUnsigned shifting") {
    {
        /*
         * Check that shifts to both sides work on single limb number
        */
        BigUnsigned a = BigUnsigned::fromBase16("F0F");

        a << 30;
        CHECK_EQ(a.toBase16(), "F0F");

        a <<= 30;
        CHECK_EQ(a.toBase16(), "3C3C0000000");

        a >>= 30;
        CHECK_EQ(a.toBase16(), "F0F");

        a >>= 9;
        CHECK_EQ(a.toBase16(), "7");

        a >>= 300;
        CHECK(a.isZero());

        a <<= 1000;
        CHECK(a.isZero());
    }

    {
        /*
         * Check that shifts to both sides work on multi limb number
        */
        std::string s = "";
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");
        s.append("F0F00F0F");

        BigUnsigned a = BigUnsigned::fromBase16(s);
        CHECK_EQ(a.limb.size(), 5);

        a <<= 23;
        std::string res = "";
        res.append("787807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87F87807");
        res.append("87800000");
        CHECK_EQ(a.toBase16(), res);
        CHECK_EQ(a.limb.size(), 5);

        a <<= 1;
        res.clear();
        res.append("F0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0FF0F00F");
        res.append("0F000000");
        CHECK_EQ(a.toBase16(), res);
        CHECK_EQ(a.limb.size(), 5);

        a >>= 24;

        CHECK_EQ(a.toBase16(), s);
        CHECK_EQ(a.limb.size(), 5);
    }

    {
        /*
         * Check that a proper shift results in new limb creation
        */
        std::string s = "";
        s.append("F0000000");
        s.append("F0000000");

        BigUnsigned a = BigUnsigned::fromBase16(s);
        CHECK_EQ(a.limb.size(), 1);

        a <<= 1;
        std::string res = "";
        res.append("1");
        res.append("E0000001");
        res.append("E0000000");

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 2);
        CHECK_EQ(a.toBase16(), res);
    }

    {
        /*
         * Check that a proper shift results in limb deletion
        */
        std::string s = "";
        s.append("0000000F");
        s.append("00000001");
        s.append("00000001");

        BigUnsigned a = BigUnsigned::fromBase16(s);
        CHECK_EQ(a.limb.size(), 2);

        a >>= 4;
        std::string res = "";
        res.append("F0000000");
        res.append("10000000");

        CHECK(!a.isZero());
        CHECK_EQ(a.limb.size(), 1);
        CHECK_EQ(a.toBase16(), res);
    }

    {
        /*
         * Check that getNBits works correctly
        */
        std::string s1 = "";
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");
        s1.append("FFFFFFFF");

        std::string s2 = "";
        s2.append("00010001");
        s2.append("00010001");
        s2.append("00010001");
        s2.append("00010001");

        BigUnsigned a = BigUnsigned::fromBase16(s1);
        BigUnsigned b = BigUnsigned::fromBase16(s2);

        CHECK_EQ(a.getNBits(), 160);
        CHECK_EQ(b.getNBits(), 113);
    }
}

TEST_CASE("BigUnsigned opeartions with uint64_t") {
    {
        std::string s = "789789789789789789";
        BigUnsigned a = BigUnsigned::fromBase16(s);

        const uint64_t val = 37;
        a += val;
        a -= val;
        a *= val;
        a /= val;

        CHECK_EQ(a.toBase16(), s);

        BigUnsigned b(100);
        CHECK(b == 100);
        CHECK(!(b != 100));
        CHECK(b > 99);
        CHECK(b < 101);
    }
}

TEST_CASE("BigUnsigned base10 and base64") {
    {
        /*
         * Check base10 conversion
        */
        std::string s = "89743891235892713957821789573821759823153253297357128571908590379531";
        BigUnsigned v = BigUnsigned::fromBase10(s);

        CHECK_EQ(v.toBase10(), s);

        s = "123";
        v = BigUnsigned::fromBase10(s);

        CHECK_EQ(v.toBase16(), "7B");
    }

    {
        /*
         * Check base64 conversion
        */
        std::string s = "fjkadhsbjkfghaklwhkgjhdkashgkjdshagjkhwekjhga83e238725y3827y317894tjkdsahfk";
        BigUnsigned v = BigUnsigned::fromBase64(s);

        CHECK_EQ(v.toBase64(), s);

        s = "aB";
        v = BigUnsigned::fromBase64(s);

        CHECK_EQ(v.toBase10(), "1665");
        CHECK_EQ(v.toBase16(), "681");
    }
}
