#include "bigunsigned.hpp"

#pragma once

enum class BaseE {
    BASE_10,
    BASE_16,
    BASE_64,
};

struct FpElement {

private:
    BigUnsigned val;
    BigUnsigned modulus;

    FpElement& add(const FpElement& other) {
        if (!inSameFieldAs(other)) throw std::runtime_error("FpElement::add elements of incompatible fields.");

        val += other.val;

        if (val >= modulus)
            val -= modulus;

        return *this;
    }

    FpElement& subtract(const FpElement& other) {
        if (!inSameFieldAs(other)) throw std::runtime_error("FpElement::subtract elements of incompatible fields.");

        *this += ~other;
        return *this;
    }

    FpElement& multiply(const FpElement& other) {
        if (!inSameFieldAs(other)) throw std::runtime_error("FpElement::multiply elements of incompatible fields.");

        val *= other.val;
        if (val >= modulus)
            val %= modulus;

        return *this;
    }

    FpElement& divide(const FpElement& other) {
        if (!inSameFieldAs(other))
            throw std::runtime_error("FpElement::divide elements of incompatible fields.");

        *this *= !other;
        return *this;
    }

    static FpElement pow(FpElement base, BigUnsigned exp) {
        FpElement res(BigUnsigned(1), base.modulus);

        while (!exp.isZero()) {
            if (exp.isOdd())
                res *= base;

            exp >>= 1;
            base *= base;
        }
        
        return res;
    }

    FpElement& neg(void) {
        if (!val.isZero())
            val = modulus - val;

        return *this;
    }

    FpElement inv(void) const {
        if (val.isZero())
            throw std::runtime_error("FpElement::inv zero is not invertible.");

        BigUnsigned exp = modulus - 2;
        return pow(*this, exp);
    }

public:
    FpElement(const BaseE b, const std::string& s1, const std::string& s2)
        : val(0), modulus(0)
    {
        switch (b) {
            case BaseE::BASE_10:
                val = BigUnsigned::fromBase10(s1);
                modulus = BigUnsigned::fromBase10(s2);
                break;

            case BaseE::BASE_64:
                val = BigUnsigned::fromBase64(s1);
                modulus = BigUnsigned::fromBase64(s2);
                break;
        
            case BaseE::BASE_16:
            default:
                val = BigUnsigned::fromBase16(s1);
                modulus = BigUnsigned::fromBase16(s2);
                break;
        }

        if (modulus.isZero())
            throw std::runtime_error("FpElement::FpElement modulus is zero.");

        if (val >= modulus)
            val %= modulus;
    }

    FpElement(const BigUnsigned& v, const BigUnsigned& m)
        : val(v), modulus(m)
    {
        if (modulus.isZero())
            throw std::runtime_error("FpElement::FpElement modulus is zero");
        if (val >= modulus)
            val %= modulus;
    }

    FpElement(const std::string& s1, const std::string& s2)
        : FpElement(BaseE::BASE_16, s1, s2) {}

    FpElement(const std::string& s, const BigUnsigned v)
        : FpElement(BaseE::BASE_16, s, v.toBase16()) {}

    FpElement() : val(0), modulus(0) {}

    bool inSameFieldAs(const FpElement& other) const {
        return modulus == other.modulus;
    }

    FpElement& operator+=(const FpElement& other) { return add(other); }
    friend FpElement operator+(FpElement a, const FpElement& b) { a += b; return a; }

    FpElement& operator-=(const FpElement& other) { return subtract(other); }
    friend FpElement operator-(FpElement a, const FpElement& b) { a -= b; return a; }

    FpElement& operator*=(const FpElement& other) { return multiply(other); }
    friend FpElement operator*(FpElement a, const FpElement& b) { a *= b; return a; }

    FpElement& operator/=(const FpElement& other) { return divide(other); }
    friend FpElement operator/(FpElement a, const FpElement& b) { a /= b; return a; }

    friend FpElement operator!(const FpElement& a) { return a.inv(); }
    friend FpElement operator~(FpElement a) { return a.neg(); }

    friend bool operator==(const FpElement& lhs, const FpElement& rhs) {
        return lhs.inSameFieldAs(rhs) && (lhs.val == rhs.val);
    }

    friend bool operator!=(const FpElement& lhs, const FpElement& rhs) {
        return !(lhs == rhs);
    }

    BigUnsigned getVal(void) const {
        return BigUnsigned(val);
    }

    BigUnsigned getMod(void) const {
        return BigUnsigned(modulus);
    }
};
