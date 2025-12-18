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

    BigUnsigned() noexcept = default;
    BigUnsigned(const BigUnsigned& v) noexcept = default;

    /* Designed for shadowing simple uint64_t */
    explicit BigUnsigned(const uint64_t v) {
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

    size_t getNBits(void) const {
        const size_t nLimbs = limb.size();
        if (nLimbs == 0) return 0;

        uint64_t msl = limb.back();
        uint8_t zeros = 0;
        const uint64_t mask = (uint64_t{1} << 63);
        while ((msl & mask) == 0) { ++zeros; msl <<= 1; }
        
        return (nLimbs - 1) * 64 + (64 - zeros);
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
     * a > b => return 1
     * a < b => return -1
     * a == b => return 0
    */
    static int compare(const BigUnsigned& a, const uint64_t b) {
        const size_t nLimbs = a.limb.size();
        
        if (nLimbs == 0) return (b == 0) ? 0 : -1;
        if (nLimbs > 1) return 1;

        if (a.limb[0] < b) return -1;
        if (a.limb[0] > b) return 1;
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

    BigUnsigned& add_small(const uint64_t other) {
        if (other == 0) return *this;
        if (isZero()) {
            limb.push_back(other);
            return *this;
        }

        const size_t nLimbs = limb.size();

        uint64_t carry = other;
        for (size_t i = 0; i < nLimbs; ++i) {
            const uint64_t iLimb = limb[i];
            const __uint128_t iSum =
                static_cast<__uint128_t>(iLimb) +
                static_cast<__uint128_t>(carry);

            limb[i] = static_cast<uint64_t>(iSum);
            carry = static_cast<uint64_t>(iSum >> 64);
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

    BigUnsigned& substract_small(const uint64_t other) {
        if (*this < other) throw std::runtime_error("BigUnsigned::substract_small result is negative.");

        uint64_t borrow = other;

        const size_t nLimbs = limb.size();
        for (size_t i = 0; i < nLimbs; ++i) {
            if (borrow == 0) break;

            const uint64_t minuend = limb[i];
            const uint64_t substrahend = borrow;
            __int128_t diff =
                static_cast<__int128_t>(minuend) -
                static_cast<__int128_t>(substrahend);

            if (diff < 0) {
                diff += (static_cast<__int128_t>(1) << 64);
                borrow = 1;   
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

    BigUnsigned& mult_small(const uint64_t other) {
        if (other == 1) return *this;
        if (other == 0) {
            limb.clear();
            return *this;
        }

        uint64_t carry = 0;
        const uint64_t multiplier = other;
        const size_t nLimbs = limb.size();
        for (size_t i = 0; i < nLimbs; ++i) {
            const uint64_t multiplicand = limb[i];
            const __uint128_t product =
                static_cast<__uint128_t>(multiplicand) *
                static_cast<__uint128_t>(multiplier) +
                static_cast<__uint128_t>(carry);

            limb[i] = static_cast<uint64_t>(product);
            carry = static_cast<uint64_t>(product >> 64);
        }

        if (carry != 0) limb.push_back(carry);
        return *this;
    }

    std::pair<BigUnsigned, BigUnsigned>
    divmod(const BigUnsigned& divisor) const {
        if (divisor.isZero()) throw std::runtime_error("BigUnsigned::divmod division by zero.");
        if (isZero()) return {BigUnsigned{0}, BigUnsigned{0}};
        if (*this < divisor) return {BigUnsigned{0}, *this};
        if (*this == divisor) return {BigUnsigned{1}, BigUnsigned{0}};

        BigUnsigned quotient(0);
        BigUnsigned remainder(*this);

        const size_t nBitsA = getNBits();
        const size_t nBitsB = divisor.getNBits();
        const size_t bitDiff = nBitsA - nBitsB;

        BigUnsigned shiftedDivisor = divisor << bitDiff;
        size_t shift = bitDiff;

        while (true) {
            if (remainder >= shiftedDivisor) {
                remainder -= shiftedDivisor;
                quotient += (BigUnsigned(1) << shift);
            }

            if (shift == 0) break;
            shiftedDivisor >>= 1;
            --shift;
        }

        quotient.normalize();
        remainder.normalize();
        return {quotient, remainder};
    }    

    uint64_t divmod_small(const uint64_t divisor) {
        if (divisor == 0) throw std::runtime_error("BigUnsigned::divmod_small division by zero.");
        if (isZero()) return 0ULL;
        if (divisor == 1) return 0ULL;
        
        __uint128_t carry = 0;
        const size_t nLimbs = limb.size();
        for (size_t i = 0; i < nLimbs; ++i) {
            const uint64_t iLimb = limb[nLimbs - i - 1];
            const __uint128_t curr = (carry << 64) | iLimb;
            limb[nLimbs - i - 1] = static_cast<uint64_t>(curr / divisor);
            carry = static_cast<uint64_t>(curr % divisor);
        }

        normalize();
        return static_cast<uint64_t>(carry);
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

        return *this;
    }

    BigUnsigned& operator>>=(const size_t bits) {
        if (bits == 0 || isZero()) return *this;

        const size_t nDelLimbs = bits / 64;
        const size_t nDelBits = bits % 64;
        const size_t nLimbsBefore = limb.size();

        if (nDelLimbs >= nLimbsBefore) {
            limb.clear();
            return *this;
        }

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

        normalize();
        return *this;
    }

    BigUnsigned& operator+=(const BigUnsigned& other) { return add(other); }
    BigUnsigned& operator+=(const uint64_t other) { return add_small(other); }
    BigUnsigned& operator-=(const BigUnsigned& other) { return substract(other); }
    BigUnsigned& operator-=(const uint64_t other) { return substract_small(other); }
    BigUnsigned& operator*=(const BigUnsigned& other) { return mult(other); }
    BigUnsigned& operator*=(const uint64_t other) { return mult_small(other); }
    BigUnsigned& operator/=(const BigUnsigned& other) {
        const auto p = divmod(other);
        *this = std::move(p.first);
        return *this;
    }
    BigUnsigned& operator/=(const uint64_t other) {
        (void) divmod_small(other);
        return *this;
    }
    BigUnsigned& operator%=(const BigUnsigned& other) {
        const auto p = divmod(other);
        *this = std::move(p.second);
        return *this;
    }
    BigUnsigned& operator%=(const uint64_t other) {
        const uint64_t rem = divmod_small(other);
        limb.clear();

        if (rem != 0) limb.push_back(rem);
        return *this;
    }
    BigUnsigned& operator=(const uint64_t other) {
        limb.clear();
        if (other != 0) limb.push_back(other);

        return *this;
    }

    friend BigUnsigned operator+(BigUnsigned a, const BigUnsigned& b) { a += b; return a; }
    friend BigUnsigned operator+(BigUnsigned a, const uint64_t b) { a += b; return a; }
    friend BigUnsigned operator+(const uint64_t a, BigUnsigned b) { b += a; return b; }
    friend BigUnsigned operator-(BigUnsigned a, const BigUnsigned& b) { a -= b; return a; }
    friend BigUnsigned operator-(BigUnsigned a, const uint64_t b) { a -= b; return a; }
    friend BigUnsigned operator*(BigUnsigned a, const BigUnsigned& b) { a *= b; return a; }
    friend BigUnsigned operator*(BigUnsigned a, const uint64_t b) { a *= b; return a; }
    friend BigUnsigned operator*(const uint64_t a, BigUnsigned b) { b *= a; return b; }
    friend BigUnsigned operator/(BigUnsigned a, const BigUnsigned& b) { a /= b; return a; }
    friend BigUnsigned operator/(BigUnsigned a, const uint64_t b) { a /= b; return a; }
    friend BigUnsigned operator%(BigUnsigned a, const BigUnsigned& b) { a %= b; return a; }
    friend uint64_t operator%(BigUnsigned a, const uint64_t b) { return a.divmod_small(b); }
    friend BigUnsigned operator<<(BigUnsigned x, const size_t bits) { x <<= bits; return x; }
    friend BigUnsigned operator>>(BigUnsigned x, const size_t bits) { x >>= bits; return x; }

    friend bool operator>(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) == 1; }
    friend bool operator>(const BigUnsigned& a, const uint64_t b) { return compare(a, b) == 1; }
    friend bool operator>(const uint64_t a, const BigUnsigned& b) { return compare(b, a) == -1; }
    friend bool operator>=(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) != -1; }
    friend bool operator>=(const BigUnsigned& a, const uint64_t b) { return compare(a, b) != -1; }
    friend bool operator>=(const uint64_t a, const BigUnsigned& b) { return compare(b, a) != 1; }
    friend bool operator<(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) == -1; }
    friend bool operator<(const BigUnsigned& a, const uint64_t b) { return compare(a, b) == -1; }
    friend bool operator<(const uint64_t a, const BigUnsigned& b) { return compare(b, a) == 1; }
    friend bool operator<=(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) != 1; }
    friend bool operator<=(const BigUnsigned& a, const uint64_t b) { return compare(a, b) != 1; }
    friend bool operator<=(const uint64_t a, const BigUnsigned& b) { return compare(b, a) != -1; }
    friend bool operator==(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) == 0; }
    friend bool operator==(const BigUnsigned& a, const uint64_t b) { return compare(a, b) == 0; }
    friend bool operator==(const uint64_t a, const BigUnsigned& b) { return compare(b, a) == 0; }
    friend bool operator!=(const BigUnsigned& a, const BigUnsigned& b) { return compare(a, b) != 0; }
    friend bool operator!=(const BigUnsigned& a, const uint64_t b) { return compare(a, b) != 0; }
    friend bool operator!=(const uint64_t a, const BigUnsigned& b) { return compare(b, a) != 0; }

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
    static BigUnsigned fromBase16(const std::string& s) {
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

                if (conv < 0) throw std::runtime_error("BigUnsigned::fromBase16 invalid character.");

                candidate |= (static_cast<uint64_t>(conv) << (nhexes * 4));
                ++nhexes;
            }
            if (nhexes > 0) res.limb.push_back(candidate);
        }
        
        res.normalize();
        return res;
    }

    std::string toBase16(void) const {
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

    static BigUnsigned fromBase10(const std::string& s) {
        BigUnsigned res(0);
        if (s.empty()) return res;

        const size_t lenOfLiteral = s.size();
        for (size_t i = 0; i < lenOfLiteral; ++i) {
            const char c = s.at(i);
            if (c < '0' || c > '9') throw std::runtime_error("BigUnsigned::fromBase10 invalid character");

            const uint64_t digit = c - '0';
            res = res * 10 + digit;
        }

        return res;
    }

    std::string toBase10(void) const {
        if (isZero()) return "0";

        BigUnsigned val(*this);
        std::string res = "";
        res.reserve(limb.size() * 20);

        while (!val.isZero()) {
            const uint8_t digit = val % 10;
            const char c = '0' + digit;
            res.push_back(c);
            val = val / 10;
        }
        
        std::reverse(res.begin(), res.end());
        return res;
    }

    static BigUnsigned fromBase64(const std::string s) {
        BigUnsigned res(0);
        if (s.empty()) return res;

        auto atob64 = [](const char c) -> int {
            if (c >= 'A' && c <= 'Z') return c - 'A';
            if (c >= 'a' && c <= 'z') return 26 + (c - 'a');
            if (c >= '0' && c <= '9') return 52 + (c - '0');
            if (c == '+') return 62;
            if (c == '/') return 63;
            return -1;
        };

        for (const char c : s) {
            const int digit = atob64(c);
            if (digit < 0) throw std::runtime_error("BigUnsigned::fromBase64 invalid character.");

            res *= 64;
            res += static_cast<uint64_t>(digit);
        }

        res.normalize();
        return res;
    }

    std::string toBase64(void) const {
        if (isZero()) return "A";

        constexpr char alphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        BigUnsigned val(*this);

        std::string res = "";
        res.reserve(val.limb.size() * 5);

        while (!val.isZero()) {
            const uint64_t idx = val % 64;
            val /= 64;

            const char sym = alphabet[idx];
            res.push_back(sym);
        }
     
        std::reverse(res.begin(), res.end());
        return res;
    }
};
