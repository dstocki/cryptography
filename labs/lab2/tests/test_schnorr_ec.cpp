#include "doctest/doctest.h"

#include "ellipticcurve.hpp"
#include "fpelement.hpp"
#include "bigunsigned.hpp"

#include "schnorr_ec.hpp"
#include "encode.hpp"
#include "sha256.hpp"

TEST_CASE("Schnorr signature on elliptic curve over Fp") {

    /*
     * Field F_97
     */
    BigUnsigned p(97);
    FpElement a(BigUnsigned(2), p);
    FpElement b(BigUnsigned(3), p);

    EllipticCurve<FpElement> curve(a, b);

    /*
     * Generator G = (3,6)
     */
    FpElement gx(BigUnsigned(3), p);
    FpElement gy(BigUnsigned(6), p);

    EllipticCurve<FpElement>::Point G(gx, gy);

    CHECK(curve.isOnCurve(G));

    /*
     * Klucz prywatny i publiczny
     */
    BigUnsigned q(5);        // porządek (mały, testowy)
    BigUnsigned x(2);        // private key

    auto Y = curve.scalarMul(x, G);  // public key

    /*
     * Nonce i wiadomość
     */
    BigUnsigned k(3);
    std::string msg = "Alice";

    /*
     * Podpis
     */
    auto sig = schnorr_ec_sign(
        curve,
        G,
        Y,
        x,
        k,
        q,
        msg
    );

    /*
     * Weryfikacja: R' = sG + eY
     */
    auto sG = curve.scalarMul(sig.s, G);
    auto eY = curve.scalarMul(sig.e, Y);
    auto Rv = curve.add(sG, eY);

    /*
     * Encode(R')
     */
    EncodedECPoint encR;
    encR.x = encodeFp(Rv.x.getVal(), 1);
    encR.y = encodeFp(Rv.y.getVal(), 1);

    std::string encodedR = encodeEC(encR);

    BigUnsigned e2 =
        sha256_to_bigunsigned(encodedR + msg) % q;

    CHECK_EQ(e2, sig.e);
}

TEST_CASE("Schnorr EC detects message modification") {

    BigUnsigned p(97);
    FpElement a(BigUnsigned(2), p);
    FpElement b(BigUnsigned(3), p);

    EllipticCurve<FpElement> curve(a, b);

    EllipticCurve<FpElement>::Point G(
        FpElement(BigUnsigned(3), p),
        FpElement(BigUnsigned(6), p)
    );

    BigUnsigned q(5);
    BigUnsigned x(2);
    auto Y = curve.scalarMul(x, G);

    BigUnsigned k(3);

    auto sig = schnorr_ec_sign(
        curve, G, Y, x, k, q, "Alice"
    );

    /*
     * Weryfikacja z inną wiadomością
     */
    auto sG = curve.scalarMul(sig.s, G);
    auto eY = curve.scalarMul(sig.e, Y);
    auto Rv = curve.add(sG, eY);

    EncodedECPoint encR;
    encR.x = encodeFp(Rv.x.getVal(), 1);
    encR.y = encodeFp(Rv.y.getVal(), 1);

    std::string encodedR = encodeEC(encR);

    BigUnsigned e2 =
        sha256_to_bigunsigned(encodedR + "Bob") % q;

    CHECK(e2 != sig.e);
}

TEST_CASE("Schnorr EC detects wrong public key") {

    BigUnsigned p(97);
    FpElement a(BigUnsigned(2), p);
    FpElement b(BigUnsigned(3), p);

    EllipticCurve<FpElement> curve(a, b);

    EllipticCurve<FpElement>::Point G(
        FpElement(BigUnsigned(3), p),
        FpElement(BigUnsigned(6), p)
    );

    BigUnsigned q(5);
    BigUnsigned x(2);
    auto Y = curve.scalarMul(x, G);

    BigUnsigned k(3);

    auto sig = schnorr_ec_sign(
        curve, G, Y, x, k, q, "Alice"
    );

    /*
     * FAŁSZYWY klucz publiczny
     */
    BigUnsigned x_fake(4);
    auto Y_fake = curve.scalarMul(x_fake, G);

    auto sG = curve.scalarMul(sig.s, G);
    auto eY = curve.scalarMul(sig.e, Y_fake);
    auto Rv = curve.add(sG, eY);

    EncodedECPoint encR;
    encR.x = encodeFp(Rv.x.getVal(), 1);
    encR.y = encodeFp(Rv.y.getVal(), 1);

    std::string encodedR = encodeEC(encR);

    BigUnsigned e2 =
        sha256_to_bigunsigned(encodedR + "Alice") % q;

    CHECK(e2 != sig.e);
}
