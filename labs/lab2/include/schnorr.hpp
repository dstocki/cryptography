#pragma once
#include <string>
#include "bigunsigned.hpp"

struct SchnorrSignature {
    BigUnsigned s;
    BigUnsigned e;
};

#include "sha256.hpp"

SchnorrSignature schnorrSign(
    const BigUnsigned& x,
    const BigUnsigned& q,
    const BigUnsigned& k,
    const std::string& encodedR,
    const std::string& message
) {
    BigUnsigned e = sha256_to_bigunsigned(encodedR + message) % q;

    BigUnsigned ex = (e * x) % q;
    BigUnsigned s;

    if (k >= ex)
        s = (k - ex) % q;
    else
        s = (k + q - ex) % q;

    return {s, e};
}

bool schnorrVerify(
    const BigUnsigned& q,
    const BigUnsigned& e,
    const std::string& encodedR,
    const std::string& message
) {
    BigUnsigned e2 = sha256_to_bigunsigned(encodedR + message) % q;
    return e2 == e;
}

