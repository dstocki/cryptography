#include "doctest/doctest.h"
#include "schnorr.hpp"
#include "encode.hpp"

TEST_CASE("Schnorr signature – example from task") {

    // p = 65537 = 0x010001 → 3 bajty
    BigUnsigned p(65537);
    size_t byteLen = 3;

    // R = 17
    BigUnsigned R(17);
    std::string encR = encodeFp(R, byteLen);

    CHECK_EQ(encR, std::string("000011"));

    std::string msg = "Alice";

    BigUnsigned x(5);      // private key
    BigUnsigned q(65537);  // order
    BigUnsigned k(123);    // nonce (testowy)

    auto sig = schnorrSign(x, q, k, encR, msg);

    CHECK(schnorrVerify(q, sig.e, encR, msg));
}

TEST_CASE("Schnorr detects message modification") {

    BigUnsigned q(65537);
    BigUnsigned x(9);
    BigUnsigned k(42);

    BigUnsigned R(21);
    std::string encR = encodeFp(R, 3);

    auto sig = schnorrSign(x, q, k, encR, "Alice");

    CHECK_FALSE(
        schnorrVerify(q, sig.e, encR, "Bob")
    );
}
