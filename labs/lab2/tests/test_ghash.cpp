#include "doctest/doctest.h"
#include "ghash.hpp"

/*
 * Testy GHASH bazują na własnościach algebraicznych,
 * a nie pełnym AES-GCM (brak szyfrowania).
 */

TEST_CASE("GHASH basic properties in GF(2^128)") {

    /*
     * Wielomian GHASH:
     * f(x) = x^128 + x^7 + x^2 + x + 1
     */
    const std::string irr =
        "1000000000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000010000111";

    /*
     * H = 1 (neutralny element mnożenia)
     */
    F2mElement H("1", irr);

    SUBCASE("Empty A and C give zero tag") {
        std::vector<uint8_t> A;
        std::vector<uint8_t> C;

        F2mElement tag = GHASH(H, A, C);

        CHECK_EQ(tag.toBitString(), std::string("0"));
    }

    SUBCASE("Only A influences the tag when C is empty") {
        std::vector<uint8_t> A = {0x01, 0x02, 0x03};
        std::vector<uint8_t> C;

        F2mElement tag1 = GHASH(H, A, C);
        F2mElement tag2 = GHASH(H, A, C);

        CHECK_EQ(tag1.toBitString(), tag2.toBitString());
    }

    SUBCASE("Changing A changes the GHASH value") {
        std::vector<uint8_t> A1 = {0x01, 0x02, 0x03};
        std::vector<uint8_t> A2 = {0x01, 0x02, 0x04};
        std::vector<uint8_t> C;

        F2mElement tag1 = GHASH(H, A1, C);
        F2mElement tag2 = GHASH(H, A2, C);

        CHECK(tag1 != tag2);
    }

    SUBCASE("Only C influences the tag when A is empty") {
        std::vector<uint8_t> A;
        std::vector<uint8_t> C = {0xAA, 0xBB, 0xCC};

        F2mElement tag1 = GHASH(H, A, C);
        F2mElement tag2 = GHASH(H, A, C);

        CHECK_EQ(tag1.toBitString(), tag2.toBitString());
    }

    SUBCASE("Changing C changes the GHASH value") {
        std::vector<uint8_t> A;
        std::vector<uint8_t> C1 = {0xAA, 0xBB, 0xCC};
        std::vector<uint8_t> C2 = {0xAA, 0xBB, 0xCD};

        F2mElement tag1 = GHASH(H, A, C1);
        F2mElement tag2 = GHASH(H, A, C2);

        CHECK(tag1 != tag2);
    }

    SUBCASE("Different H produces different GHASH") {
        F2mElement H2("10", irr); // H = x

        std::vector<uint8_t> A = {0x01, 0x02};
        std::vector<uint8_t> C = {0xFF};

        F2mElement tag1 = GHASH(H, A, C);
        F2mElement tag2 = GHASH(H2, A, C);

        CHECK(tag1 != tag2);
    }
}
