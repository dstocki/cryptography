/* --------------------------------------------------
 * Subject: Cryptography
 * List: Lab 1
 * Task: Task 1
 * Author: Denis Stocki
 * --------------------------------------------------
*/

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <inttypes.h>

/*
    +-----------------------------------------------------------------------------------+
    | k-th limb: A_k_63 A_k_62 ... A_k_1 A_k_0                                          |
    |   * MSB_k => A_k_63                                                               |
    |   * LSB_k => A_k_0                                                                |
    |   * MSB_{k + 1} > LSB_{k + 1} > MSB_k > LSB_k                                     |
    |   * limb[k][i] = A_k_i = 2^(64 * k + i)                                           |
    |                                                                                   |
    | n-th bit's limb is d=floor(n / 64)                                                |
    | n-th bit's position in limb d is l=(n % 64)                                       |
    |                                                                                   |
    | Example 1:                                                                        |
    | n=72 (72th bit indexed from 1)                                                    |
    |                                                                                   |
    | limb[0] = 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 |
    | limb[1] = 00000000 00000000 00000000 00000000 00000000 00000000 00000000 10000000 |
    | limb[2] = ...                                                                     |
    | limb[3] = ...                                                                     |
    |                                                                                   |
    | Example 2:                                                                        |
    | x=2^88 + 12 (value)                                                               |
    |                                                                                   |
    | limb[0] = 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00001100 |
    | limb[1] = 00000000 00000000 00000000 00000000 00000001 00000000 00000000 00000000 |
    | limb[2] = ...                                                                     |
    +-----------------------------------------------------------------------------------+

*/
struct BigUnsigned {
    std::vector<uint64_t> limb;

    BigUnsigned() = default;

    /* Designed for shadowing simple uint64_t */
    BigUnsigned(const uint64_t v) {
        if (v != 0) limb.push_back(v);
    }

    bool isZero(void) const { // const, because does not modify "this"
        return limb.empty();
    }

    bool isOne(void) const {
        if (limb.size() == 1) {
            return (limb[0] == 1ULL);
        } else {
            return false;
        }
    }

    /* Brief: Removes all MSLs such that MSL == 0 */
    void normalize(void) {
        while (!limb.empty() && limb.back() == 0) {
            limb.pop_back();
        }
    }

    /*
     * a > b => return 1
     * a < b => return -1
     * a == b => return 0
    */
    static int compare(const BigUnsigned& a, const BigUnsigned& b) {
        const size_t loa = a.limb.size(); // Length of a
        const size_t lob = b.limb.size();

        if (loa != lob) {
            return (loa > lob) ? 1 : -1;
        }

        for (size_t i = loa; i-- > 0;) {
            const uint64_t voa = a.limb[i]; // Value of a
            const uint64_t vob = b.limb[i];

            if (voa > vob) return 1;
            if (voa < vob) return -1;
        }

        return 0;
    }

    /*
        +--------------------------------------------------+
        | Let a_i be i-th limb of BigUnsigned a            |
        | Let b_i be i-th limb of BigUnsigned b            |
        |                                                  |
        | this = a_0 a_1 ... a_m                           |
        | other = b_0 b_1 ... b_n                          |
        |                                                  |
        | 1. Find max(m, n) and resize this                |
        | 2. Initialize carry with 0                       |
        | 3. Addition begins from limbs a_0 and b_0        |
        | 4. Move carry from addition i to addition i + 1  |
        | 5. If, after max(m, n) additions carry != 0 then |
        |    create a new limb in this with this carry     |
        +--------------------------------------------------+
    */
    BigUnsigned& add(const BigUnsigned& other) {
        const size_t lothis = limb.size();
        const size_t loothr = other.limb.size();
        const size_t lbase = std::max(lothis, loothr);
        limb.resize(lbase, 0);

        uint64_t carry = 0;
        for (size_t limidx = 0; limidx < lbase; ++limidx) {
            const uint64_t elemOfA = (limidx < loothr) ? other.limb[limidx] : 0ULL;
            const __uint128_t sum =
                static_cast<__uint128_t>(limb[limidx]) + elemOfA + carry;
            limb[limidx] = static_cast<uint64_t>(sum);
            carry = static_cast<uint64_t>(sum >> 64);
        }
        
        if (carry != 0) limb.push_back(carry);
        return *this;
    }

    BigUnsigned& substract(const BigUnsigned& other) {
        const size_t lenOfThis = limb.size();
        const size_t lenOfOthr = other.limb.size();

        if (*this < other) throw std::runtime_error("BigUnsigned::substract: result is negative");
        
        uint64_t borrow = 0;
        for (size_t i = 0; i < lenOfThis; ++i) {
            const uint64_t minuend = limb[i];
            const uint64_t substrahend = (i < lenOfOthr) ? other.limb[i] : 0ULL;
            __int128_t diff =
                static_cast<__int128_t>(minuend) -
                static_cast<__int128_t>(substrahend) -
                static_cast<__int128_t>(borrow);

            if (diff < 0) {
                borrow = 1;
                diff += (static_cast<__int128_t>(1) << 64);
            } else {
                borrow = 0;
            }
            
            limb[i] = static_cast<uint64_t>(diff);
        }

        normalize();
        return *this;
    }

    BigUnsigned& mult(const BigUnsigned& other) {
        if (isZero() || other.isZero()) {
            limb.clear();
            return *this;
        }

        const size_t lenoft = limb.size();
        const size_t lenofoth = other.limb.size();
        std::vector<uint64_t> res(lenoft + lenofoth, 0);

        for (size_t i = 0; i < lenoft; ++i) {
            __uint128_t carry = 0;
            for (size_t j = 0; j < lenofoth; ++j) {
                const uint64_t otherElem = other.limb[j];
                const __uint128_t sum =
                    static_cast<__uint128_t>(limb[i]) *
                    static_cast<__uint128_t>(otherElem) +
                    static_cast<__uint128_t>(res[i + j]) +
                    carry;
                res[i + j] = static_cast<uint64_t>(sum);
                carry = (sum >> 64);
            }
            if (carry != 0) {
                res[i + lenofoth] += static_cast<uint64_t>(carry);
            }
        }

        limb.swap(res);
        normalize();
        return *this;
    }

    BigUnsigned& operator<<=(const size_t bits) {
        if (isZero() || bits == 0) return *this;

        const size_t nNewLimbs = bits / 64;
        const size_t nNewBits = bits % 64;

        if (nNewLimbs > 0) limb.insert(limb.begin(), nNewLimbs, 0);
        if (nNewBits != 0) {
            const size_t nLimbs = limb.size();

            uint64_t carry = 0;
            for (size_t i = nNewLimbs; i < nLimbs; ++i) {
                const uint64_t val = limb[i];
                limb[i] = (val << nNewBits) | carry;
                carry = (val >> (64 - nNewBits));
            }

            if (carry != 0) limb.push_back(carry);
        }

        normalize();
        return *this;
    }

    BigUnsigned& operator>>=(const size_t bits) {
        if (bits == 0 || isZero()) return *this;

        const size_t nDelLimbs = bits / 64;
        const size_t nDelBits = bits % 64;
        const size_t nLimbsBefore = limb.size();

        if (nDelLimbs >= nLimbsBefore) limb.erase(limb.begin(), limb.end());
        else {
            if (nDelLimbs > 0) limb.erase(limb.begin(), limb.begin() + nDelLimbs);
            if (nDelBits != 0) {
                const size_t nLimbsAfter = limb.size();
                uint64_t carry = 0;
                for (size_t i = nLimbsAfter; i-- > 0; ) {
                    const uint64_t val = limb[i];
                    limb[i] = (val >> nDelBits) | carry;
                    carry = val << (64 - nDelBits);
                }
            }
        }

        normalize();
        return *this;
    }

    BigUnsigned& operator+=(const BigUnsigned& other) { return add(other); }
    BigUnsigned& operator-=(const BigUnsigned& other) { return substract(other); }
    BigUnsigned& operator*=(const BigUnsigned& other) { return mult(other); }

    friend BigUnsigned operator+(BigUnsigned a, const BigUnsigned& b) { a += b; return a; }
    friend BigUnsigned operator-(BigUnsigned a, const BigUnsigned& b) { a -= b; return a; }
    friend BigUnsigned operator*(BigUnsigned a, const BigUnsigned& b) { a *= b; return a; }
    friend BigUnsigned operator<<(BigUnsigned x, const size_t bits) { x <<= bits; return x; }
    friend BigUnsigned operator>>(BigUnsigned x, const size_t bits) { x >>= bits; return x; }

    friend bool operator>(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) == 1; }
    friend bool operator>=(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) != -1; }
    friend bool operator<(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) == -1; }
    friend bool operator<=(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) != 1; }
    friend bool operator==(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) == 0; }
    friend bool operator!=(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) != 0; }

    /*
        +--------------------------------------------------------------------------------------------------+
        | s = "f777777777777777777777777777777775"                                                         |
        |   * MSH = f                                                                                      |
        |   * LSH = 5                                                                                      |
        |   * Each hex is made of 4 bits                                                                   |
        |   * Each limb is made of 16 hexes                                                                |
        |                                                                                                  |
        | fullLimb[0] = "7777777777777775"                                                                 |
        | => limb[0] = (0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0101)_2 |
        |                                                                                                  |
        | fullLimb[1] = "7777777777777777"                                                                 |
        | => limb[1] = (0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111 0111)_2 |
        |                                                                                                  |
        | partialLimb = "f7"                                                                               |
        | => limb[2] = (0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1111 0111)_2 |
        +--------------------------------------------------------------------------------------------------+
    */
    static BigUnsigned fromHex(const std::string& s) {
        BigUnsigned res;
        res.limb.clear();

        auto ahxtoi = [](const char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            else if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
            else if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
            else return -1;
        };

        const size_t los = s.size();
        if (los == 0) return res;

        size_t idx = los;
        while (idx > 0) {
            size_t nhexes = 0;
            uint64_t candidate = 0;
            while (idx > 0 && nhexes < 16) {
                const char c = s.at(--idx);
                const int conv = ahxtoi(c);

                if (conv < 0) return BigUnsigned();

                candidate |= (static_cast<uint64_t>(conv) << (nhexes * 4));
                ++nhexes;
            }
            if (nhexes > 0) res.limb.push_back(candidate);
        }
        
        res.normalize();
        return res;
    }

    std::string toHex(void) const {
        if (limb.empty()) return "0";

        const char alphabet[] = "0123456789ABCDEF";

        std::ostringstream oss;

        const size_t nooflim = limb.size();
        size_t limidx = nooflim;
        while (limidx > 0) {
            const uint64_t l = limb.at(--limidx);
            char buff[16];

            for (size_t hx = 0; hx < 16; ++hx) {
                const uint64_t nibble = ((l >> (4 * hx)) & 0xFULL); // Value from 0 to 15
                const char hxa = alphabet[nibble];
                buff[15 - hx] = hxa;
            }

            if (limidx == (nooflim - 1)) {
                size_t offset = 0;
                while (offset < 15 && buff[offset] == '0') ++offset;
                
                oss.write(buff + offset, 16 - offset);
            } else {
                oss.write(buff, 16);
            }
        }

        return oss.str();
    }
};
