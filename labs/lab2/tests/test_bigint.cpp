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
        CHECK(a.limb.size() == 1);
    }

    {
        /*
            Check that U64 limb constructor cannot receive value higher than U64_max
            and such attempt results in overflow
        */
        BigInt a(UINT64_MAX + 1);
        CHECK(a.toHex() == "0");
    }
}

TEST_CASE("BigInt fromHex/toHex and normalization function") {
    {
        /*
         * Check that 0's hex string is treated as no limbs
        */
       BigInt a = BigInt::fromHex("0000000000");
       CHECK(a.isZero());
       CHECK(a.toHex() == "0");
    }

    {
        /*
         * Check that normalization erases all 0's before a non-zero value
        */
       BigInt a = BigInt::fromHex("0000123");
       CHECK(!a.isZero());
       CHECK(a.toHex() == "123");
       CHECK(a.limb.size() == 1);
    }

    {
        /*
         * Check that non hex digit destroys constructor and returns empty BigInt
        */
       BigInt a = BigInt::fromHex("11111G111");
       CHECK(a.isZero());
       CHECK(a.toHex() == "0");
    }

    {
        /*
         * Check that toHex removes leading zeros in MSL
        */
       BigInt a = BigInt::fromHex("002001");
       CHECK(!a.isZero());
       CHECK(a.limb.size() == 1);
       CHECK(a.toHex() == "2001");
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
       CHECK(a.limb.size() == 3); // Without normalization would be 5

       std::string res = "";
       res.append("100");
       res.append("00000000");
       res.append("00000010");
       res.append("00000000");
       res.append("00000001");
       CHECK(a.toHex() == res);
    }
}
