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
struct BigInt {
    std::vector<uint64_t> limb;

    BigInt() = default;

    /* Designed for shadowing simple uint64_t */
    BigInt(const uint64_t v) {
        if (v != 0) limb.push_back(v);
    }

    bool isZero(void) const { // const, because does not modify "this"
        return limb.empty();
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
     * a ==b => return 0
    */
    static int compare(const BigInt& a, const BigInt& b) {
        const size_t loa = a.limb.size(); // Length of a
        const size_t lob = b.limb.size();

        if (loa != lob) {
            return (loa > lob) ? 1 : -1;
        }

        for (size_t i = (loa - 1); i >= 0; --i) {
            const uint64_t voa = a.limb[i]; // Value of a
            const uint64_t vob = b.limb[i];

            if (voa > vob) return 1;
            if (voa < vob) return -1;
        }

        return 0;
    }

    friend bool operator>(const BigInt& a, const BigInt& b) { return compare(a, b) == 1; }
    friend bool operator>=(const BigInt& a, const BigInt& b) { return compare(a, b) != -1; }
    friend bool operator<(const BigInt& a, const BigInt& b) { return compare(a, b) == -1; }
    friend bool operator<=(const BigInt& a, const BigInt& b) { return compare(a, b) != 1; }
    friend bool operator==(const BigInt& a, const BigInt& b) { return compare(a, b) == 0; }
    friend bool operator!=(const BigInt& a, const BigInt& b) { return compare(a, b) != 0; }
};
