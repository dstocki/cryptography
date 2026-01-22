#pragma once
#include <string>
#include "bigunsigned.hpp"

#include <openssl/sha.h>

inline BigUnsigned sha256_to_bigunsigned(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        hash
    );

    BigUnsigned res(0);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        res <<= 8;
        res += hash[i];
    }
    return res;
}

