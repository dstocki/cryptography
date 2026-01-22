#pragma once
#include "bigunsigned.hpp"
#include "ellipticcurve.hpp"
#include "sha256.hpp"
#include "encode.hpp"

template <typename FieldT>
struct SchnorrECSignature {
    BigUnsigned s;
    BigUnsigned e;
};

template <typename FieldT>
inline SchnorrECSignature<FieldT> schnorr_ec_sign(
    const EllipticCurve<FieldT>& curve,
    const typename EllipticCurve<FieldT>::Point& G,
    const typename EllipticCurve<FieldT>::Point& Y,
    const BigUnsigned& x,
    const BigUnsigned& k,
    const BigUnsigned& q,
    const std::string& msg
) {
    // R = k * G
    auto R = curve.scalarMul(k, G);

    // długość bajtowa = długość modułu pola
    size_t byteLen =
        (R.x.getMod().getNBits() + 7) / 8;

    // Encode(R)
    EncodedECPoint encR;
    encR.x = encodeFp(R.x.getVal(), byteLen);
    encR.y = encodeFp(R.y.getVal(), byteLen);

    std::string encodedR = encodeEC(encR);

    BigUnsigned e =
        sha256_to_bigunsigned(encodedR + msg) % q;

    BigUnsigned s;
    if (k >= (e * x) % q)
        s = (k - (e * x) % q) % q;
    else
        s = (k + q - (e * x) % q) % q;

    return {s, e};
}
