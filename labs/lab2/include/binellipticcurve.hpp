#pragma once

#include "f2melement.hpp"
#include "bigunsigned.hpp"

// y^2 + x y = x^3 + a x^2 + b over F_{2^m}
template <typename FieldT>
class BinaryEllipticCurve {
public:
    struct Point {
        FieldT x;
        FieldT y;
        bool infinity;

        Point() : x(), y(), infinity(true) {}
        Point(const FieldT& x_, const FieldT& y_)
            : x(x_), y(y_), infinity(false) {}
    };

private:
    FieldT a;
    FieldT b;

    static FieldT zeroFrom(const FieldT& sample) {
        FieldT z = sample;
        z -= sample; // v - v = 0
        return z;
    }

    static bool isZero(const FieldT& v) {
        FieldT z = zeroFrom(v);
        return v == z;
    }

    Point doublePoint(const Point& P) const {
        if (P.infinity) return P;

        FieldT zeroX = zeroFrom(P.x);
        FieldT zeroY = zeroFrom(P.y);

        // y == 0 -> 2P = O
        if (P.y == zeroY)
            return Point();

        // x == 0 -> inf
        if (P.x == zeroX)
            return Point();

        FieldT x1 = P.x;
        FieldT y1 = P.y;

        // L = x1 + y1 / x1
        FieldT y1_over_x1 = y1 / x1;
        FieldT lambda = x1;
        lambda += y1_over_x1;

        // x3 = L^2 + L + a
        FieldT lambda2 = lambda * lambda;
        FieldT x3 = lambda2;
        x3 += lambda;
        x3 += a;

        // y3 = x1^2 + (L + 1) x3
        FieldT x1sq = x1 * x1;

        // (L + 1)
        FieldT one = b;
        one /= b; // 1 as b/b (b != 0 for nonsingular curve)
        FieldT lamPlusOne = lambda;
        lamPlusOne += one;

        FieldT term = lamPlusOne * x3;
        FieldT y3 = x1sq;
        y3 += term;

        return Point(x3, y3);
    }

public:
    using ECPoint = Point;

    BinaryEllipticCurve(const FieldT& a_, const FieldT& b_)
        : a(a_), b(b_) {
        // b != 0 (nonsingularity)
        if (isZero(b))
            throw std::runtime_error("BinaryEllipticCurve: parameter b must be non-zero.");
    }

    Point infinity(void) const { return Point(); }

    bool isOnCurve(const Point& P) const {
        if (P.infinity) return true;

        const FieldT& x = P.x;
        const FieldT& y = P.y;

        FieldT lhs = y * y;
        FieldT xy  = x * y;
        lhs += xy; // y^2 + xy

        FieldT x2 = x * x;
        FieldT x3 = x2 * x;
        FieldT ax2 = a * x2;

        FieldT rhs = x3;
        rhs += ax2;
        rhs += b; // x^3 + a x^2 + b

        return lhs == rhs;
    }

    Point negate(const Point& P) const {
        if (P.infinity) return P;

        // -P = (x, x + y) in characteristics 2
        FieldT newY = P.x;
        newY += P.y;
        return Point(P.x, newY);
    }

    Point add(const Point& P, const Point& Q) const {
        if (P.infinity) return Q;
        if (Q.infinity) return P;

        const FieldT& x1 = P.x;
        const FieldT& y1 = P.y;
        const FieldT& x2 = Q.x;
        const FieldT& y2 = Q.y;

        if (x1 == x2) {
            if (y1 == y2) {
                // P == Q doubling
                return doublePoint(P);
            } else {
                // F_2^m on vertical line are P and -P,
                // if x1 == x2 i y1 != y2 -> Q = -P, sum = O
                return Point();
            }
        }

        // L = (y1 + y2) / (x1 + x2)
        FieldT num = y1;
        num += y2;

        FieldT den = x1;
        den += x2;

        FieldT lambda = num / den;

        // x3 = L^2 + L + x1 + x2 + a
        FieldT lambda2 = lambda * lambda;

        FieldT x3 = lambda2;
        x3 += lambda;
        x3 += x1;
        x3 += x2;
        x3 += a;

        // y3 = L(x1 + x3) + x3 + y1
        FieldT x1_plus_x3 = x1;
        x1_plus_x3 += x3;

        FieldT y3 = lambda * x1_plus_x3;
        y3 += x3;
        y3 += y1;

        return Point(x3, y3);
    }

    Point scalarMul(const BigUnsigned& k, const Point& P) const {
        Point R = infinity();
        Point Q = P;
        BigUnsigned n = k;

        while (!n.isZero()) {
            if (n.isOdd()) {
                R = add(R, Q);
            }
            n >>= 1;
            if (!n.isZero()) {
                Q = add(Q, Q);
            }
        }

        return R;
    }
};
