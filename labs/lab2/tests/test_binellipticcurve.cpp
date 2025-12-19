#include "doctest/doctest.h"
#include "f2melement.hpp"
#include "binellipticcurve.hpp"

// F_{2^4} with irreducible f(x) = x^4 + x + 1 -> "10011"

TEST_CASE("BinaryEllipticCurve: find points on curve and basic properties") {
    const std::string irr = "10011";

    F2mElement a("0010", irr); // x
    F2mElement b("0001", irr); // 1
    BinaryEllipticCurve<F2mElement> E(a, b);
    using Point = BinaryEllipticCurve<F2mElement>::Point;

    // P != O on the curve
    Point P;
    bool foundP = false;

    for (unsigned xv = 0; xv < 16 && !foundP; ++xv) {
        for (unsigned yv = 0; yv < 16 && !foundP; ++yv) {
            std::string xs, ys;
            for (int i = 3; i >= 0; --i) {
                xs.push_back( ((xv >> i) & 1u) ? '1' : '0' );
                ys.push_back( ((yv >> i) & 1u) ? '1' : '0' );
            }

            F2mElement xElem(xs, irr);
            F2mElement yElem(ys, irr);
            Point cand(xElem, yElem);

            if (E.isOnCurve(cand)) {
                P = cand;
                foundP = true;
            }
        }
    }

    CHECK(foundP);
    CHECK(E.isOnCurve(P));

    // Point in infinity
    Point O = E.infinity();
    CHECK(O.infinity);
    CHECK(E.isOnCurve(O));

    // P + O = O + P = P
    Point R1 = E.add(P, O);
    Point R2 = E.add(O, P);

    CHECK(!R1.infinity);
    CHECK(!R2.infinity);
    CHECK(R1.x == P.x);
    CHECK(R1.y == P.y);
    CHECK(R2.x == P.x);
    CHECK(R2.y == P.y);

    // P + (-P) = O
    Point minusP = E.negate(P);
    Point sumP = E.add(P, minusP);
    CHECK(sumP.infinity);
    CHECK(E.isOnCurve(sumP));

    // 2P = P + P
    Point twoP = E.add(P, P);
    CHECK(E.isOnCurve(twoP));

    // 3P = P + 2P
    Point threeP = E.add(P, twoP);
    CHECK(E.isOnCurve(threeP));

    // 3P = scalarMul(3, P)
    BigUnsigned three(3);
    Point threeP_fast = E.scalarMul(three, P);

    CHECK(threeP_fast.infinity == threeP.infinity);
    if (!threeP_fast.infinity) {
        CHECK(threeP_fast.x == threeP.x);
        CHECK(threeP_fast.y == threeP.y);
    }
}

TEST_CASE("BinaryEllipticCurve: scalar multiplication vs repeated addition") {
    const std::string irr = "10011";

    F2mElement a("0010", irr); // x
    F2mElement b("0001", irr); // 1
    BinaryEllipticCurve<F2mElement> E(a, b);
    using Point = BinaryEllipticCurve<F2mElement>::Point;

    // Find point P on curve
    Point P;
    bool foundP = false;

    for (unsigned xv = 0; xv < 16 && !foundP; ++xv) {
        for (unsigned yv = 0; yv < 16 && !foundP; ++yv) {
            std::string xs, ys;
            for (int i = 3; i >= 0; --i) {
                xs.push_back(((xv >> i) & 1u) ? '1' : '0');
                ys.push_back(((yv >> i) & 1u) ? '1' : '0');
            }

            F2mElement xElem(xs, irr);
            F2mElement yElem(ys, irr);
            Point cand(xElem, yElem);

            if (E.isOnCurve(cand)) {
                P = cand;
                foundP = true;
            }
        }
    }

    CHECK(foundP);
    CHECK(E.isOnCurve(P));

    for (int kInt = 0; kInt <= 10; ++kInt) {
        BigUnsigned k(static_cast<uint64_t>(kInt));

        Point fast = E.scalarMul(k, P);

        // Repeated addition
        Point slow = E.infinity();
        for (int i = 0; i < kInt; ++i)
            slow = E.add(slow, P);

        CHECK(fast.infinity == slow.infinity);
        if (!fast.infinity) {
            CHECK(fast.x == slow.x);
            CHECK(fast.y == slow.y);
        }
    }
}
