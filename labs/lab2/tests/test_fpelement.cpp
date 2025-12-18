#include "doctest/doctest.h"
#include "bigunsigned.hpp"
#include "fpelement.hpp"

TEST_CASE("FpElement constructors and normalization") {
    {
        /*
         * Check that value is reduced modulo p in constructor (BASE_10)
         *  a = 10 mod 7  →  a = 3
        */
        FpElement a(BaseE::BASE_10, "10", "7");
        FpElement expected(BaseE::BASE_10, "3", "7");
        CHECK(a == expected);
    }

    {
        /*
         * Check that value is reduced modulo p in constructor (BASE_16)
         *  a = 0x10 mod 0x7 → a = 0x3
        */
        FpElement a(BaseE::BASE_16, "11", "7");
        FpElement expected(BaseE::BASE_16, "3", "7");
        CHECK(a == expected);
    }

    {
        /*
         * Check that 0 element is represented correctly (BASE_10)
        */
        FpElement zero(BaseE::BASE_10, "0", "7");
        FpElement alsoZero(BaseE::BASE_10, "7", "7");
        CHECK(zero == alsoZero);
    }

    {
        /*
         * Check that constructor with (string, BigUnsigned) keeps the same field
        */
        BigUnsigned p = BigUnsigned::fromBase16("7");
        FpElement a("5", p);
        FpElement b(BaseE::BASE_16, "5", "7");
        CHECK(a == b);
    }
}

TEST_CASE("FpElement addition and subtraction in F_7") {
    /*
     * We work in F_7; all results are taken modulo 7.
     */
    FpElement zero(BaseE::BASE_10, "0", "7");
    FpElement one (BaseE::BASE_10, "1", "7");
    FpElement two (BaseE::BASE_10, "2", "7");
    FpElement three(BaseE::BASE_10, "3", "7");
    FpElement five (BaseE::BASE_10, "5", "7");

    {
        /*
         * Check that 5 + 3 = 1 (mod 7)
        */
        FpElement a = five;
        FpElement b = three;
        a += b;
        CHECK(a == one);
    }

    {
        /*
         * Check that 3 - 5 = 5 (mod 7), because 3 - 5 ≡ -2 ≡ 5 (mod 7)
        */
        FpElement a = three;
        FpElement b = five;
        a -= b;
        CHECK(a == five);
    }

    {
        /*
         * Check that a + (-a) = 0 (mod 7)
        */
        FpElement a = five;
        FpElement negA = ~a;
        FpElement sum = a + negA;
        CHECK(sum == zero);
    }

    {
        /*
         * Check that adding element from different field throws
         */
        FpElement a(BaseE::BASE_10, "3", "7");
        FpElement b(BaseE::BASE_10, "3", "11");

        CHECK_THROWS_WITH_MESSAGE(
            a += b,
            "FpElement::add elements of incompatible fields.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("FpElement multiplication and division in F_7") {
    FpElement zero(BaseE::BASE_10, "0", "7");
    FpElement one (BaseE::BASE_10, "1", "7");
    FpElement two (BaseE::BASE_10, "2", "7");
    FpElement three(BaseE::BASE_10, "3", "7");
    FpElement four(BaseE::BASE_10, "4", "7");
    FpElement five(BaseE::BASE_10, "5", "7");

    {
        /*
         * Check that 3 * 5 = 1 (mod 7)
         */
        FpElement a = three;
        FpElement b = five;
        a *= b;
        CHECK(a == one);
    }

    {
        /*
         * Check that 2 * 4 = 1 (mod 7) because 2 * 4 = 8 ≡ 1
         */
        FpElement a = two;
        FpElement b = four;
        FpElement c = a * b;
        CHECK(c == one);
    }

    {
        /*
         * Check that a / b = a * b^{-1} (mod 7)
         */
        FpElement a = five;
        FpElement b = three;
        FpElement div1 = a / b;

        FpElement invB = !b;
        FpElement div2 = a * invB;

        CHECK(div1 == div2);
    }

    {
        /*
         * Check that division by zero (non-invertible element) throws
         */
        FpElement a = three;
        FpElement zeroElem = zero;

        CHECK_THROWS_WITH_MESSAGE(
            a /= zeroElem,
            "FpElement::inv zero is not invertible.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("FpElement inverse and negation") {
    FpElement one (BaseE::BASE_10, "1", "7");
    FpElement two (BaseE::BASE_10, "2", "7");
    FpElement three(BaseE::BASE_10, "3", "7");
    FpElement four(BaseE::BASE_10, "4", "7");
    FpElement five(BaseE::BASE_10, "5", "7");
    FpElement six (BaseE::BASE_10, "6", "7");
    FpElement zero(BaseE::BASE_10, "0", "7");

    {
        /*
         * Check that !a is multiplicative inverse: a * !a = 1 (mod 7)
        */
        FpElement inv2 = !two;
        FpElement prod = two * inv2;
        CHECK(prod == one);
    }

    {
        /*
         * Check that inv(5) = 3 in F_7  (5 * 3 = 15 ≡ 1)
        */
        FpElement inv5 = !five;
        CHECK(inv5 == three);
        FpElement prod = five * inv5;
        CHECK(prod == one);
    }

    {
        /*
         * Check that additive inverse satisfies a + (~a) = 0 (mod 7)
        */
        FpElement neg4 = ~four;
        FpElement sum  = four + neg4;
        CHECK(sum == zero);
    }

    {
        /*
         * Check that negation of 0 is 0
        */
        FpElement neg0 = ~zero;
        CHECK(neg0 == zero);
    }

    {
        /*
         * Check that inv(0) throws
        */
        CHECK_THROWS_WITH_MESSAGE(
            (!zero),
            "FpElement::inv zero is not invertible.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("FpElement operations in different bases") {
    {
        /*
         * Check that same element constructed in BASE_10 and BASE_16 is equal
         *    value = 15, modulus = 101
        */
        FpElement a(BaseE::BASE_10, "15", "101");
        FpElement b(BaseE::BASE_16, "F",  "65");

        CHECK(a == b);
    }

    {
        /*
         * Check that BASE_64 constructor gives the same value as BASE_10/16,
         *     using consistency of BigUnsigned::fromBase64
         *
         * Take some random base64 string, build two elements with same modulus
         * and check that addition works consistently.
        */
        std::string s64 = "aB";
        FpElement a(BaseE::BASE_64, "aB",  "bB");
        FpElement b(BaseE::BASE_16, "681", "6C1");

        CHECK(a == b);

        FpElement zero(BaseE::BASE_10, "0", "1729");
        FpElement c = a + zero;
        CHECK(c == a);
    }
}
