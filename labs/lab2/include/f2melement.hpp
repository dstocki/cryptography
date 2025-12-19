#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>
#include "bigunsigned.hpp"

// Bit i in BigUnsigned is coeff nearby x^i.
struct F2mElement {
private:
    BigUnsigned val;     // coeffs a(x)
    BigUnsigned modPoly; // irreducible f(x)
    std::size_t m;       // deg f(x): m = deg f

    static BigUnsigned fromBits(const std::string& bits) {
        BigUnsigned res(0);
        for (char c : bits) {
            if (c != '0' && c != '1')
                throw std::runtime_error("F2mElement::fromBits invalid bit character.");
            res <<= 1;
            if (c == '1')
                res += 1u;
        }
        return res;
    }

    static std::string toBits(const BigUnsigned& v) {
        if (v.isZero())
            return "0";

        BigUnsigned tmp = v;
        std::string out;
        while (!tmp.isZero()) {
            uint64_t bit = tmp.limb.empty() ? 0 : (tmp.limb[0] & 1u);
            out.push_back(bit ? '1' : '0');
            tmp >>= 1;
        }
        std::reverse(out.begin(), out.end()); // LSB->MSB into MSB->LSB
        return out;
    }

    static void xorInto(BigUnsigned& a, const BigUnsigned& b) {
        std::size_t n = std::max(a.limb.size(), b.limb.size());
        a.limb.resize(n, 0);

        for (std::size_t i = 0; i < b.limb.size(); ++i)
            a.limb[i] ^= b.limb[i];

        while (!a.limb.empty() && a.limb.back() == 0)
            a.limb.pop_back();
    }

    static std::size_t degree(const BigUnsigned& x) {
        if (x.isZero())
            return 0;
        return x.getNBits() - 1;
    }

    // carry-less multiply
    static BigUnsigned mulPoly(const BigUnsigned& a, const BigUnsigned& b) {
        BigUnsigned res(0);
        BigUnsigned x = a;
        BigUnsigned y = b;

        while (!y.isZero()) {
            if (!y.limb.empty() && (y.limb[0] & 1u))
                xorInto(res, x);

            x <<= 1;  // shift x -> x * X
            y >>= 1;
        }
        return res;
    }

    BigUnsigned reduce(const BigUnsigned& rIn) const {
        if (modPoly.isZero())
            throw std::runtime_error("F2mElement::reduce modulus polynomial is zero.");

        BigUnsigned r = rIn;

        std::size_t degMod = degree(modPoly); // m
        if (degMod == 0)
            throw std::runtime_error("F2mElement::reduce invalid modulus degree.");

        while (!r.isZero()) {
            std::size_t degR = degree(r);
            if (degR < degMod)
                break;

            std::size_t shift = degR - degMod;
            BigUnsigned shifted = modPoly;
            shifted <<= shift;

            xorInto(r, shifted);
        }

        return r;
    }

public:
    // "1011" -> 1*x^3 + 0*x^2 + 1*x + 1
    // "10011" -> x^4 + x + 1
    F2mElement(const std::string& bits, const std::string& irrBits) {
        val     = fromBits(bits);
        modPoly = fromBits(irrBits);

        if (modPoly.isZero())
            throw std::runtime_error("F2mElement::F2mElement modulus polynomial is zero.");

        m = degree(modPoly); // deg(f)

        if (m == 0)
            throw std::runtime_error("F2mElement::F2mElement modulus polynomial degree must be >= 1.");

        val = reduce(val);
    }

    F2mElement(const BigUnsigned& v, const BigUnsigned& irr)
        : val(v), modPoly(irr) {
        if (modPoly.isZero())
            throw std::runtime_error("F2mElement::F2mElement modulus polynomial is zero.");
        m = degree(modPoly);
        if (m == 0)
            throw std::runtime_error("F2mElement::F2mElement modulus polynomial degree must be >= 1.");
        val = reduce(val);
    }

    F2mElement()
        : val(0), modPoly(0), m(0) {}

    std::string toBitString(void) const {
        return toBits(val);
    }

    std::string modulusToBitString(void) const {
        return toBits(modPoly);
    }

    std::size_t degreeM(void) const { return m; }

    F2mElement& operator+=(const F2mElement& other) {
        if (modPoly != other.modPoly)
            throw std::runtime_error("F2mElement::operator+= incompatible fields.");

        xorInto(val, other.val);
        return *this;
    }

    F2mElement& operator-=(const F2mElement& other) {
        return (*this += other); // -b = b
    }

    F2mElement& operator*=(const F2mElement& other) {
        if (modPoly != other.modPoly)
            throw std::runtime_error("F2mElement::operator*= incompatible fields.");

        BigUnsigned prod = mulPoly(val, other.val);
        val = reduce(prod);
        return *this;
    }

    // in F_2 -a = a
    F2mElement operator-(void) const {
        return *this;
    }

    static F2mElement pow(F2mElement base, BigUnsigned exp) {
        F2mElement one(BigUnsigned(1), base.modPoly);
        F2mElement res = one;

        while (!exp.isZero()) {
            if (!exp.limb.empty() && (exp.limb[0] & 1u))
                res *= base;
            exp >>= 1;
            if (!exp.isZero())
                base *= base;
        }
        return res;
    }

    // a^{2^m - 2}
    F2mElement inv(void) const {
        if (val.isZero())
            throw std::runtime_error("F2mElement::inv zero is not invertible.");

        // q = 2^m
        BigUnsigned q(1);
        q <<= m;         // q = 1 << m (2^m)

        BigUnsigned exp = q;
        exp -= 2u;       // q - 2

        return pow(*this, exp);
    }

    F2mElement& operator/=(const F2mElement& other) {
        if (modPoly != other.modPoly)
            throw std::runtime_error("F2mElement::operator/= incompatible fields.");

        F2mElement invB = other.inv();
        *this *= invB;
        return *this;
    }

    friend F2mElement operator+(F2mElement a, const F2mElement& b) {
        a += b;
        return a;
    }

    friend F2mElement operator-(F2mElement a, const F2mElement& b) {
        a -= b;
        return a;
    }

    friend F2mElement operator*(F2mElement a, const F2mElement& b) {
        a *= b;
        return a;
    }

    friend F2mElement operator/(F2mElement a, const F2mElement& b) {
        a /= b;
        return a;
    }

    friend bool operator==(const F2mElement& lhs, const F2mElement& rhs) {
        return lhs.modPoly == rhs.modPoly && lhs.val == rhs.val;
    }

    friend bool operator!=(const F2mElement& lhs, const F2mElement& rhs) {
        return !(lhs == rhs);
    }

    BigUnsigned getValRaw(void) const { return val; }
    BigUnsigned getModPolyRaw(void) const { return modPoly; }
};
