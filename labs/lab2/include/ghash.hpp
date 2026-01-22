#pragma once
#include <vector>
#include <cstdint>

#include "f2melement.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

/*
 * Wielomian nieredukowalny dla GHASH (AES-GCM):
 * f(x) = x^128 + x^7 + x^2 + x + 1
 *
 * Postać bitowa (MSB -> LSB), 129 bitów
 */
static const std::string GHASH_POLY =
    "1000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000010000111";

/*
 * Padding zerami do wielokrotności 128 bitów (16 bajtów)
 */
static std::vector<uint8_t> padWithZeros(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> out = data;
    while (out.size() % 16 != 0)
        out.push_back(0x00);
    return out;
}

/*
 * Dopisanie długości w bitach (64-bit big-endian)
 */
static void appendLength(std::vector<uint8_t>& data, uint64_t bitlen) {
    for (int i = 7; i >= 0; --i)
        data.push_back(static_cast<uint8_t>((bitlen >> (8 * i)) & 0xFF));
}

/*
 * Konwersja 16 bajtów (128 bitów, big-endian) -> F2mElement
 */
static F2mElement blockToF2m(const uint8_t block[16]) {
    std::string bits;
    bits.reserve(128);

    for (int i = 0; i < 16; ++i) {
        for (int b = 7; b >= 0; --b) {
            bits.push_back((block[i] & (1 << b)) ? '1' : '0');
        }
    }

    return F2mElement(bits, GHASH_POLY);
}

/*
 * Implementacja GHASH(H, A, C)
 */
F2mElement GHASH(
    const F2mElement& H,
    const std::vector<uint8_t>& A,
    const std::vector<uint8_t>& C
) {
    // 1. S = A || pad(A) || C || pad(C) || len(A) || len(C)
    std::vector<uint8_t> S;

    auto Ap = padWithZeros(A);
    auto Cp = padWithZeros(C);

    S.insert(S.end(), Ap.begin(), Ap.end());
    S.insert(S.end(), Cp.begin(), Cp.end());

    appendLength(S, static_cast<uint64_t>(A.size()) * 8);
    appendLength(S, static_cast<uint64_t>(C.size()) * 8);

    // 2. X0 = 0 (w tym samym ciele co H)
    F2mElement X(BigUnsigned(0), H.getModPolyRaw());

    // 3. Iteracja GHASH
    for (size_t i = 0; i < S.size(); i += 16) {
        uint8_t block[16];
        std::copy(S.begin() + i, S.begin() + i + 16, block);

        F2mElement Si = blockToF2m(block);
        X = (X + Si) * H;   // XOR + mnożenie w GF(2^128)
    }

    return X;
}

