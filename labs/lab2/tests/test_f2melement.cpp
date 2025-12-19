#include "doctest/doctest.h"
#include "f2melement.hpp"

// F_{2^4} with irreducible f(x) = x^4 + x + 1  ->  bitstring "10011"

TEST_CASE("F2mElement constructors and reduction") {
    const std::string irr = "10011";

    {
        /*
         * Check that zero is represented correctly and degree is set
         */
        F2mElement z("0", irr);
        CHECK_EQ(z.toBitString(), std::string("0"));
        CHECK_EQ(z.degreeM(), 4);
        CHECK_EQ(z.modulusToBitString(), irr);
    }

    {
        /*
         * Check that x^4 reduces to x + 1
         *  x^4 ≡ x + 1 (mod x^4 + x + 1)
         *  x + 1 -> bits "11"
         */
        F2mElement x4("10000", irr);      // x^4
        CHECK_EQ(x4.toBitString(), std::string("11"));
    }

    {
        /*
         * Check that x^5 reduces to x^2 + x.
         *  x^5 = x * x^4 ≡ x * (x + 1) = x^2 + x
         *  x^2 + x -> bits "110"
         */
        F2mElement x5("100000", irr);     // x^5
        CHECK_EQ(x5.toBitString(), std::string("110"));
    }
}

TEST_CASE("F2mElement addition and subtraction in F_2^4") {
    const std::string irr = "10011"; // x^4 + x + 1

    F2mElement a("1010", irr); // a(x) = x^3 + x
    F2mElement b("0111", irr); // b(x) = x^2 + x + 1

    {
        /*
         * (x^3 + x) + (x^2 + x + 1) = x^3 + x^2 + 1
         * bits: 1010 xor 0111 = 1101
         */
        F2mElement c = a + b;
        CHECK_EQ(c.toBitString(), std::string("1101"));
    }

    {
        /*
         * In F_2: subtraction = addition
         * a - b same as a + b
         */
        F2mElement c1 = a + b;
        F2mElement c2 = a - b;
        CHECK_EQ(c1.toBitString(), c2.toBitString());
    }

    {
        /*
         * Adding zero does not change the value
         */
        F2mElement zero("0", irr);
        F2mElement c1 = a + zero;
        F2mElement c2 = a - zero;

        CHECK_EQ(c1.toBitString(), a.toBitString());
        CHECK_EQ(c2.toBitString(), a.toBitString());
    }

    {
        /*
         * Check that adding elements from incompatible fields throws
         * x^4 + 1  -> "10001"
         */
        F2mElement c("1", "10001");

        CHECK_THROWS_WITH_MESSAGE(
            a += c,
            "F2mElement::operator+= incompatible fields.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("F2mElement multiplication in F_2^4") {
    const std::string irr = "10011"; // x^4 + x + 1

    F2mElement x ("0010", irr); // x
    F2mElement x2("0100", irr); // x^2
    F2mElement x3("1000", irr); // x^3

    {
        /*
         * x * x = x^2
         * expected: "100"
         */
        F2mElement c = x * x;
        CHECK_EQ(c.toBitString(), std::string("100"));
    }

    {
        /*
         * x^2 * x^2 = x^4 ≡ x + 1
         * expected: "11"
         */
        F2mElement c = x2 * x2;
        CHECK_EQ(c.toBitString(), std::string("11"));
    }

    {
        /*
         * (x + 1)(x^2 + 1) = x^3 + x^2 + x + 1
         * expected: "1111"
         */
        F2mElement a("0011", irr); // x + 1
        F2mElement b("0101", irr); // x^2 + 1

        F2mElement c = a * b;
        CHECK_EQ(c.toBitString(), std::string("1111"));
    }

    {
        /*
         * x^3 * x = x^4 ≡ x + 1
         * expected: "11"
         */
        F2mElement c = x3 * x;
        CHECK_EQ(c.toBitString(), std::string("11"));
    }

    {
        /*
         * Multiplication in incompatible fields should throw
         */
        F2mElement a("1010", irr);
        F2mElement b("1010", "10001");

        CHECK_THROWS_WITH_MESSAGE(
            a *= b,
            "F2mElement::operator*= incompatible fields.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("F2mElement inverse and division in F_2^4") {
    const std::string irr = "10011"; // x^4 + x + 1

    F2mElement one("1", irr);
    F2mElement zero("0", irr);

    F2mElement a("0010", irr);  // x
    F2mElement b("1011", irr);  // x^3 + x + 1

    {
        /*
         * a * a^{-1} = 1
         */
        F2mElement invA = a.inv();
        F2mElement prod = a * invA;

        CHECK_EQ(prod.toBitString(), one.toBitString());
    }

    {
        /*
         * b * b^{-1} = 1
         */
        F2mElement invB = b.inv();
        F2mElement prod = b * invB;

        CHECK_EQ(prod.toBitString(), one.toBitString());
    }

    {
        /*
         * a / b * b = a
         */
        F2mElement q = a / b;
        F2mElement back = q * b;

        CHECK_EQ(back.toBitString(), a.toBitString());
    }

    {
        /*
         * Division by zero should throw
         */
        F2mElement c("0011", irr); // x + 1

        CHECK_THROWS_WITH_MESSAGE(
            c /= zero,
            "F2mElement::inv zero is not invertible.",
            "std::runtime_error"
        );
    }
}

TEST_CASE("F2mElement exponentiation") {
    const std::string irr = "10011"; // x^4 + x + 1

    F2mElement one("1", irr);
    F2mElement zero("0", irr);
    F2mElement a("1010", irr); // x^3 + x

    {
        /*
         * a^0 = 1
         */
        BigUnsigned exp0(0);
        F2mElement r0 = F2mElement::pow(a, exp0);
        CHECK_EQ(r0.toBitString(), one.toBitString());
    }

    {
        /*
         * a^1 = a
         */
        BigUnsigned exp1(1);
        F2mElement r1 = F2mElement::pow(a, exp1);
        CHECK_EQ(r1.toBitString(), a.toBitString());
    }

    {
        /*
         * a^5 z fast pow == a^5
         */
        BigUnsigned exp5(5);
        F2mElement fast = F2mElement::pow(a, exp5);

        F2mElement slow = one;
        for (int i = 0; i < 5; ++i)
            slow *= a;

        CHECK_EQ(fast.toBitString(), slow.toBitString());
    }

    {
        /*
         * 0^0 = 1
         * 0^k dla k>0 = 0
         */
        BigUnsigned e0(0);
        BigUnsigned e3(3);

        F2mElement r00 = F2mElement::pow(zero, e0);
        F2mElement r03 = F2mElement::pow(zero, e3);

        CHECK_EQ(r00.toBitString(), one.toBitString());
        CHECK_EQ(r03.toBitString(), zero.toBitString());
    }
}
