#include "doctest/doctest.h"
#include "ellipticcurve.hpp"
#include "fpelement.hpp"

TEST_CASE("EllipticCurve over F_11: points on curve and group law") {
    /*
     * Curve y^2 = x^3 + 2x + 7 over F_11
     */
    FpElement a(BaseE::BASE_10, "2", "11");
    FpElement b(BaseE::BASE_10, "7", "11");
    EllipticCurve<FpElement> E(a, b);
    using Point = EllipticCurve<FpElement>::Point;

    // P = (6, 2)
    FpElement xP(BaseE::BASE_10, "6", "11");
    FpElement yP(BaseE::BASE_10, "2", "11");
    Point P(xP, yP);

    // Q = (7, 1)
    FpElement xQ(BaseE::BASE_10, "7", "11");
    FpElement yQ(BaseE::BASE_10, "1", "11");
    Point Q(xQ, yQ);

    CHECK(E.isOnCurve(P));
    CHECK(E.isOnCurve(Q));

    /*
     * P + Q = (10, 2)
     */
    FpElement xR(BaseE::BASE_10, "10", "11");
    FpElement yR(BaseE::BASE_10, "2", "11");
    Point R_expected(xR, yR);

    Point R = E.add(P, Q);
    CHECK(E.isOnCurve(R));
    CHECK(R.x == R_expected.x);
    CHECK(R.y == R_expected.y);

    /*
     * Double point 2P = (10, 9)
     */
    FpElement x2P(BaseE::BASE_10, "10", "11");
    FpElement y2P(BaseE::BASE_10, "9", "11");
    Point twoP_expected(x2P, y2P);

    Point twoP = E.add(P, P);
    CHECK(E.isOnCurve(twoP));
    CHECK(twoP.x == twoP_expected.x);
    CHECK(twoP.y == twoP_expected.y);

    /*
     * P + (-P) = O
     */
    Point minusP = E.negate(P);
    Point O = E.add(P, minusP);
    CHECK(O.infinity);
    CHECK(E.isOnCurve(O));

    /*
     * Check 5P: fast scalar multiplication vs naive
     */
    BigUnsigned k(5);
    Point fast = E.scalarMul(k, P);

    Point slow = E.infinity();
    for (int i = 0; i < 5; ++i)
        slow = E.add(slow, P);

    CHECK(fast.infinity == slow.infinity);
    if (!fast.infinity) {
        CHECK(!slow.infinity);
        CHECK(fast.x == slow.x);
        CHECK(fast.y == slow.y);
    }
}

TEST_CASE("EllipticCurve identity element and scalar 0") {
    FpElement a(BaseE::BASE_10, "2", "11");
    FpElement b(BaseE::BASE_10, "7", "11");
    EllipticCurve<FpElement> E(a, b);
    using Point = EllipticCurve<FpElement>::Point;

    FpElement xP(BaseE::BASE_10, "6", "11");
    FpElement yP(BaseE::BASE_10, "2", "11");
    Point P(xP, yP);

    /*
     * 0 * P = O
     */
    BigUnsigned zeroK(0);
    Point O = E.scalarMul(zeroK, P);
    CHECK(O.infinity);
    CHECK(E.isOnCurve(O));

    /*
     * P + O = O + P = P
     */
    Point O2 = E.infinity();
    Point R1 = E.add(P, O2);
    Point R2 = E.add(O2, P);

    CHECK(!R1.infinity);
    CHECK(!R2.infinity);
    CHECK(R1.x == P.x);
    CHECK(R1.y == P.y);
    CHECK(R2.x == P.x);
    CHECK(R2.y == P.y);
}

TEST_CASE("EllipticCurve associativity sanity check") {
    FpElement a(BaseE::BASE_10, "2", "11");
    FpElement b(BaseE::BASE_10, "7", "11");
    EllipticCurve<FpElement> E(a, b);
    using Point = EllipticCurve<FpElement>::Point;

    Point P(
        FpElement(BaseE::BASE_10, "6", "11"),
        FpElement(BaseE::BASE_10, "2", "11")
    );
    Point Q(
        FpElement(BaseE::BASE_10, "7", "11"),
        FpElement(BaseE::BASE_10, "1", "11")
    );
    Point R(
        FpElement(BaseE::BASE_10, "10", "11"),
        FpElement(BaseE::BASE_10, "2", "11")
    ); // R = P + Q

    Point left  = E.add(E.add(P, Q), R);
    Point right = E.add(P, E.add(Q, R));

    CHECK(left.infinity == right.infinity);
    if (!left.infinity) {
        CHECK(!right.infinity);
        CHECK(left.x == right.x);
        CHECK(left.y == right.y);
    }
}
