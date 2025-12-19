#include "doctest/doctest.h"
#include "bigunsigned.hpp"
#include "fpelement.hpp"
#include "fpkelement.hpp"

/*
 * F_7 and F_7[x]/(x^2 + 1).
 */

/*
 * Irreducible M(x) = x^2 + 1 nad F_7.
 */
static std::vector<FpElement> make_modpoly_F7_x2_plus_1() {
    FpElement m0(BaseE::BASE_10, "1", "7"); // stała 1
    FpElement m1(BaseE::BASE_10, "0", "7"); // 0 * x
    FpElement m2(BaseE::BASE_10, "1", "7"); // 1 * x^2
    return { m0, m1, m2 };
}

/*
 * One from F_7[x]/(x^2+1)
 */
static FpkElement make_one_F7ext() {
    auto modPoly = make_modpoly_F7_x2_plus_1();
    return FpkElement({ "1" }, modPoly);
}

/*
 * Zero from F_7[x]/(x^2+1)
 */
static FpkElement make_zero_F7ext() {
    auto modPoly = make_modpoly_F7_x2_plus_1();
    return FpkElement::zero(modPoly);
}

TEST_CASE("FpkElement constructors and normalization") {
    {
        /*
         * Check that zero() creates the same element as explicit { "0" }.
         */
        auto modPoly = make_modpoly_F7_x2_plus_1();

        FpkElement z1 = FpkElement::zero(modPoly);
        FpkElement z2({ "0" }, modPoly); // 0 + 0*x

        CHECK(z1 == z2);
    }

    {
        /*
         * Check that x^2 == -1 in F_7[x]/(x^2 + 1).
         *
         * Element given as coeffs: [0, 0, 1] -> 0 + 0*x + 1*x^2
         * After reduction modulo x^2 + 1 we should get -1 ≡ 6 (mod 7).
         */
        auto modPoly = make_modpoly_F7_x2_plus_1();

        FpkElement x2({ "0", "0", "1" }, modPoly); // x^2
        FpkElement minus_one({ "6" }, modPoly); // -1 ≡ 6

        CHECK(x2 == minus_one);
    }

    {
        /*
         * Check that x^3 == -x in F_7[x]/(x^2 + 1).
         *
         * x^3 = x * x^2 = x * (-1) = -x.
         * -x ≡ 6*x  (-1 ≡ 6 mod 7).
         */
        auto modPoly = make_modpoly_F7_x2_plus_1();

        FpkElement x3({ "0", "0", "0", "1" }, modPoly); // x^3
        FpkElement minus_x({ "0", "6" }, modPoly); // 0 + 6*x

        CHECK(x3 == minus_x);
    }
}

TEST_CASE("FpkElement addition and subtraction in F_7[x]/(x^2 + 1)") {
    auto modPoly = make_modpoly_F7_x2_plus_1();

    FpkElement zero = make_zero_F7ext();
    FpkElement a({ "3", "5" }, modPoly); // a(x) = 3 + 5x
    FpkElement b({ "2", "6" }, modPoly); // b(x) = 2 + 6x

    {
        /*
         * Check that a + b = (3+2, 5+6) = (5, 11) ≡ (5, 4) in F_7.
         */
        FpkElement c = a + b;
        FpkElement expected({ "5", "4" }, modPoly);

        CHECK(c == expected);
    }

    {
        /*
         * Check that a - b = (3-2, 5-6) = (1, -1) ≡ (1, 6) in F_7.
         */
        FpkElement c = a - b;
        FpkElement expected({ "1", "6" }, modPoly);

        CHECK(c == expected);
    }

    {
        /*
         * Check that a + 0 = a and a - 0 = a
         */
        FpkElement c1 = a + zero;
        FpkElement c2 = a - zero;

        CHECK(c1 == a);
        CHECK(c2 == a);
    }

    {
        /*
         * Check that adding elements of incompatible fields throws
         */
        // x^2 + 2
        FpElement m0b(BaseE::BASE_10, "2", "7");
        FpElement m1b(BaseE::BASE_10, "0", "7");
        FpElement m2b(BaseE::BASE_10, "1", "7");
        std::vector<FpElement> modPoly2 = { m0b, m1b, m2b };

        FpkElement c({ "1", "1" }, modPoly2);

        CHECK_THROWS_WITH_MESSAGE(
            a += c,
            "FpkElement::operator+= incompatible fields.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("FpkElement multiplication, inverse and division in F_7[x]/(x^2 + 1)") {
    auto modPoly = make_modpoly_F7_x2_plus_1();

    FpkElement one  = make_one_F7ext();
    FpkElement zero = make_zero_F7ext();

    // a(x) = 3 + 5x
    FpkElement a({ "3", "5" }, modPoly);
    // b(x) = 2 + 4x
    FpkElement b({ "2", "4" }, modPoly);

    {
        /*
         * Check multiplication:
         * (3 + 5x)(2 + 4x) = 6 + 22x + 20x^2
         *   6 ≡ 6           (mod 7)
         *  22 ≡ 1           (mod 7)
         *  20 ≡ 6           (mod 7)
         * x^2 ≡ -1 ≡ 6      (mod 7)
         *  => 20x^2 ≡ 6 * 6 = 36 ≡ 1
         *  => res = (6 + 1) + 1*x = 7 + x ≡ 0 + x
         * x -> (0, 1).
         */
        FpkElement c = a * b;
        FpkElement expected({ "0", "1" }, modPoly);

        CHECK(c == expected);
    }

    {
        /*
         * Check that a * 1 = a and b * 1 = b
         */
        FpkElement c1 = a * one;
        FpkElement c2 = one * b;

        CHECK(c1 == a);
        CHECK(c2 == b);
    }

    {
        /*
         * Check multiplicative inverse: a * a^{-1} = 1
         */
        FpkElement invA = a.inv();
        FpkElement prod = a * invA;

        CHECK(prod == one);
    }

    {
        /*
         * Check division: (a / b) * b = a
         */
        FpkElement q = a / b;
        FpkElement back = q * b;

        CHECK(back == a);
    }

    {
        /*
         * Division by zero should throw.
         */
        FpkElement z = zero;

        CHECK_THROWS_WITH_MESSAGE(
            a /= z,
            "FpkElement::inv zero is not invertible.",
            "std::runtime_error"
        );
    }

    {
        /*
         * Multiplication in incompatible fields should throw.
         */
        // x^2 + 2
        FpElement m0b(BaseE::BASE_10, "2", "7");
        FpElement m1b(BaseE::BASE_10, "0", "7");
        FpElement m2b(BaseE::BASE_10, "1", "7");
        std::vector<FpElement> modPoly2 = { m0b, m1b, m2b };

        FpkElement c({ "1", "1" }, modPoly2);

        CHECK_THROWS_WITH_MESSAGE(
            a *= c,
            "FpkElement::operator*= incompatible fields.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("FpkElement exponentiation") {
    auto modPoly = make_modpoly_F7_x2_plus_1();

    FpkElement one  = make_one_F7ext();
    FpkElement zero = make_zero_F7ext();
    FpkElement a({ "3", "5" }, modPoly);

    {
        /*
         * Check a^0 = 1 (identity element).
         */
        BigUnsigned exp0(0);
        FpkElement r0 = FpkElement::pow(a, exp0);
        CHECK(r0 == one);
    }

    {
        /*
         * Check a^1 = a.
         */
        BigUnsigned exp1(1);
        FpkElement r1 = FpkElement::pow(a, exp1);
        CHECK(r1 == a);
    }

    {
        /*
         * Check that fast exponentiation agrees with repeated multiplication
         * for some small exponent, e.g. 13.
         */
        BigUnsigned exp13(13);
        FpkElement fast = FpkElement::pow(a, exp13);

        FpkElement slow = one;
        for (int i = 0; i < 13; ++i)
            slow *= a;

        CHECK(fast == slow);
    }

    {
        /*
         * Check that 0^0 is defined here as 1 (consistent with pow implementation),
         * but 0^k for k>0 gives 0.
         */
        BigUnsigned exp0(0);
        BigUnsigned exp5(5);

        FpkElement r00 = FpkElement::pow(zero, exp0);
        FpkElement r05 = FpkElement::pow(zero, exp5);

        CHECK(r00 == one);
        CHECK(r05 == zero);
    }
}
