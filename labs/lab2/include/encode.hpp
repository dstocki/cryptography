#pragma once
#include <string>
#include "bigunsigned.hpp"
#include "f2melement.hpp"

// (d) EC point → compact JSON
struct EncodedECPoint {
    std::string x;
    std::string y;
};

#include <algorithm>

// pomocnicze: hex z paddingiem
inline static std::string toHex(const BigUnsigned& x, size_t byteLen) {
    std::string h = x.toBase16();
    if (h.size() < byteLen * 2)
        h = std::string(byteLen * 2 - h.size(), '0') + h;
    return h;
}

// (a) Encode Fp
inline std::string encodeFp(const BigUnsigned& x, size_t byteLen) {
    return toHex(x, byteLen);
}

// (b) Encode F2m
inline std::string encodeF2m(const F2mElement& x, size_t m) {
    size_t byteLen = (m + 7) / 8;
    return toHex(x.getValRaw(), byteLen);
}

// (d) Encode EC → compact JSON
inline std::string encodeEC(const EncodedECPoint& P) {
    return std::string("{\"x\":\"") + P.x + "\",\"y\":\"" + P.y + "\"}";
}

