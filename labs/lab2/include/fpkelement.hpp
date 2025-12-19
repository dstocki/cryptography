#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include "bigunsigned.hpp"
#include "fpelement.hpp"

struct FpkElement {
    using Coeff = FpElement;

private:
    // a(x) = coeffs[0] + coeffs[1] x + ... + coeffs[d] x^d
    std::vector<Coeff> coeffs;

    // Irreducible polynomial M(x) = m0 + m1 x + ... + mk x^k
    std::vector<Coeff> modulusPoly;

    void normalize() {
        while (!coeffs.empty() && coeffs.back().getVal().isZero())
            coeffs.pop_back();
    }

    bool sameFieldAs(const FpkElement& other) const {
        return modulusPoly == other.modulusPoly;
    }

    static std::vector<Coeff> polyAddRaw(
        const std::vector<Coeff>& a,
        const std::vector<Coeff>& b)
    {
        const std::size_t n = std::max(a.size(), b.size());
        std::vector<Coeff> res;
        res.reserve(n);

        for (std::size_t i = 0; i < n; ++i) {
            if (i < a.size() && i < b.size()) {
                Coeff c = a[i] + b[i];
                res.push_back(c);
            }
            else if (i < a.size()) { res.push_back(a[i]); }
            else { res.push_back(b[i]); }
        }
        return res;
    }

    static std::vector<Coeff> polySubRaw(
        const std::vector<Coeff>& a,
        const std::vector<Coeff>& b)
    {
        const std::size_t n = std::max(a.size(), b.size());
        std::vector<Coeff> res;
        res.reserve(n);

        for (std::size_t i = 0; i < n; ++i) {
            if (i < a.size() && i < b.size()) {
                Coeff c = a[i] - b[i];
                res.push_back(c);
            } else if (i < a.size()) {
                res.push_back(a[i]);
            } else {
                Coeff c = ~b[i];
                res.push_back(c);
            }
        }
        return res;
    }

    static std::vector<Coeff> polyMulRaw(
        const std::vector<Coeff>& a,
        const std::vector<Coeff>& b)
    {
        if (a.empty() || b.empty())
            return {};

        const std::size_t n = a.size();
        const std::size_t m = b.size();

        BigUnsigned p = a[0].getMod();
        Coeff zeroCoeff(BigUnsigned(0), p);

        std::vector<Coeff> res(n + m - 1, zeroCoeff);

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < m; ++j) {
                Coeff term = a[i] * b[j];
                res[i + j] += term;
            }
        }
        return res;
    }

    std::vector<Coeff> polyMod(
        const std::vector<Coeff>& poly)
    const {
        if (modulusPoly.size() < 2)
            throw std::runtime_error("FpkElement::polyMod modulus polynomial too small.");

        std::vector<Coeff> r = poly;
        const std::size_t k = modulusPoly.size() - 1;

        while (r.size() >= modulusPoly.size()) {
            std::size_t degR = r.size() - 1;
            std::size_t shift = degR - k;

            Coeff leadCoeff = r.back();

            for (std::size_t i = 0; i < modulusPoly.size(); ++i) {
                std::size_t idx = i + shift;
                Coeff toSub = modulusPoly[i] * leadCoeff;
                r[idx] -= toSub;
            }

            while (!r.empty() && r.back().getVal().isZero())
                r.pop_back();
        }
        return r;
    }

public:
    FpkElement(
        const std::vector<Coeff>& coeffs_,
        const std::vector<Coeff>& modulusPoly_)
    : coeffs(coeffs_),
      modulusPoly(modulusPoly_)
    {
        if (modulusPoly.size() < 2)
            throw std::runtime_error("FpkElement::FpkElement modulus polynomial degree must be >= 1.");

        coeffs = polyMod(coeffs);
        normalize();
    }

    FpkElement(
        const Coeff& c0,
        const std::vector<Coeff>& modulusPoly_)
    : coeffs(1, c0),
      modulusPoly(modulusPoly_)
    {
        if (modulusPoly.size() < 2)
            throw std::runtime_error("FpkElement::FpkElement modulus polynomial degree must be >= 1.");

        coeffs = polyMod(coeffs);
        normalize();
    }

    FpkElement(std::initializer_list<const char*> coeffStrs,
                 const std::vector<Coeff>& modulusPoly_)
        : modulusPoly(modulusPoly_) {
        if (modulusPoly.size() < 2)
            throw std::runtime_error("FpkElement::FpkElement modulus polynomial degree must be >= 1.");

        BigUnsigned p = modulusPoly[0].getMod();
        std::string pHex = p.toBase16();

        for (auto s : coeffStrs) {
            coeffs.emplace_back(BaseE::BASE_16, std::string(s), pHex);
        }

        coeffs = polyMod(coeffs);
        normalize();
    }

    FpkElement(const char* s0,
                 const std::vector<Coeff>& modulusPoly_)
        : FpkElement({s0}, modulusPoly_) {}

    static FpkElement zero(
        const std::vector<Coeff>& modulusPoly)
    {
        BigUnsigned p = modulusPoly[0].getMod();
        Coeff z(BigUnsigned(0), p);
        return FpkElement(z, modulusPoly);
    }

    std::size_t degreeK(void) const {
        return modulusPoly.size() - 1;
    }

    const std::vector<Coeff>& getCoeffs(void) const { return coeffs; }
    const std::vector<Coeff>& getModPoly(void) const { return modulusPoly; }

    FpkElement& operator+=(const FpkElement& other) {
        if (!sameFieldAs(other))
            throw std::runtime_error("FpkElement::operator+= incompatible fields.");

        coeffs = polyAddRaw(coeffs, other.coeffs);
        normalize();
        return *this;
    }

    FpkElement& operator-=(const FpkElement& other) {
        if (!sameFieldAs(other))
            throw std::runtime_error("FpkElement::operator-= incompatible fields.");

        coeffs = polySubRaw(coeffs, other.coeffs);
        normalize();
        return *this;
    }

    FpkElement& operator*=(const FpkElement& other) {
        if (!sameFieldAs(other))
            throw std::runtime_error("FpkElement::operator*= incompatible fields.");

        auto prod = polyMulRaw(coeffs, other.coeffs);
        coeffs = polyMod(prod);
        normalize();
        return *this;
    }

    FpkElement operator-(void) const {
        std::vector<Coeff> neg;
        neg.reserve(coeffs.size());
        for (std::size_t i = 0; i < coeffs.size(); ++i) {
            neg.push_back(~coeffs[i]);
        }
        return FpkElement(neg, modulusPoly);
    }

    static FpkElement pow(FpkElement base, BigUnsigned exp) {
        BigUnsigned p = base.modulusPoly[0].getMod();
        Coeff oneCoeff(BigUnsigned(1), p);
        FpkElement res(oneCoeff, base.modulusPoly);

        while (!exp.isZero()) {
            if ((exp % 2u) == 1u)
                res *= base;

            exp >>= 1;
            if (!exp.isZero())
                base *= base;
        }
        return res;
    }

    FpkElement inv(void) const {
        if (coeffs.empty())
            throw std::runtime_error("FpkElement::inv zero is not invertible.");

        BigUnsigned p = modulusPoly[0].getMod();

        BigUnsigned q(1);
        const std::size_t k = degreeK();
        for (std::size_t i = 0; i < k; ++i)
            q *= p;

        BigUnsigned exp = q - 2;
        return pow(*this, exp);
    }

    FpkElement& operator/=(const FpkElement& other) {
        if (!sameFieldAs(other))
            throw std::runtime_error("FpkElement::operator/= incompatible fields.");

        *this *= other.inv();
        return *this;
    }

    friend FpkElement operator+(FpkElement a, const FpkElement& b) { a += b; return a; }
    friend FpkElement operator-(FpkElement a, const FpkElement& b) { a -= b; return a; }
    friend FpkElement operator*(FpkElement a, const FpkElement& b) { a *= b; return a; }
    friend FpkElement operator/(FpkElement a, const FpkElement& b) { a /= b;  return a; }

    friend bool operator==(const FpkElement& lhs, const FpkElement& rhs) {
        return lhs.sameFieldAs(rhs) && lhs.coeffs == rhs.coeffs;
    }
    friend bool operator!=(const FpkElement& lhs, const FpkElement& rhs) {
        return !(lhs == rhs);
    }
};
