#pragma once

#include "bigunsigned.hpp"
#include "fpelement.hpp"

template <typename FieldT>
class EllipticCurve {
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
        z -= sample;
        return z;
    }

    static bool isZero(const FieldT& v) {
        FieldT z = zeroFrom(v);
        return v == z;
    }

    // Double point P (assume P != O)
    Point doublePoint(const Point& P) const {
        if (P.infinity) return P;

        FieldT zeroY = zeroFrom(P.y);
        if (P.y == zeroY) {
            // res => inf
            return Point();
        }

        FieldT x1 = P.x;
        FieldT y1 = P.y;

        // 3 * x1^2 + a
        FieldT x1sq = x1 * x1;
        FieldT three_x1sq = x1sq;
        three_x1sq += x1sq;
        three_x1sq += x1sq;

        FieldT num = three_x1sq;
        num += a;

        // 2 * y1
        FieldT two_y1 = y1;
        two_y1 += y1;

        FieldT lambda = num / two_y1;

        FieldT lambda2 = lambda * lambda;
        FieldT x3 = lambda2;
        x3 -= x1;
        x3 -= x1;

        FieldT x1_minus_x3 = x1;
        x1_minus_x3 -= x3;
        FieldT y3 = lambda * x1_minus_x3;
        y3 -= y1;

        return Point(x3, y3);
    }

public:
    using ECPoint = Point;

    EllipticCurve(const FieldT& a_, const FieldT& b_)
        : a(a_), b(b_) {
    }

    Point infinity(void) const { return Point(); }

    bool isOnCurve(const Point& P) const {
        if (P.infinity) return true;

        FieldT lhs = P.y * P.y;

        FieldT x2 = P.x * P.x;
        FieldT x3 = x2 * P.x;

        FieldT rhs = x3;
        FieldT ax = a * P.x;
        rhs += ax;
        rhs += b;

        return lhs == rhs;
    }

    Point negate(const Point& P) const {
        if (P.infinity) return P;

        FieldT zeroY = zeroFrom(P.y);
        FieldT negY = zeroY;
        negY -= P.y; // 0 - y = -y

        return Point(P.x, negY);
    }

    Point add(const Point& P, const Point& Q) const {
        if (P.infinity) return Q;
        if (Q.infinity) return P;

        // x1 == x2
        if (P.x == Q.x) {
            FieldT sumY = P.y;
            sumY += Q.y;

            if (isZero(sumY)) {
                // P + (-P) = O
                return Point();
            } else {
                // P == Q
                return doublePoint(P);
            }
        }

        // P != Q, x1 != x2
        FieldT x1 = P.x, y1 = P.y;
        FieldT x2 = Q.x, y2 = Q.y;

        FieldT num = y2;
        num -= y1; // y2 - y1

        FieldT den = x2;
        den -= x1; // x2 - x1

        FieldT lambda = num / den;

        FieldT lambda2 = lambda * lambda;
        FieldT x3 = lambda2;
        x3 -= x1;
        x3 -= x2;

        FieldT x1_minus_x3 = x1;
        x1_minus_x3 -= x3;
        FieldT y3 = lambda * x1_minus_x3;
        y3 -= y1;

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
