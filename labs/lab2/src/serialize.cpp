#include <iostream>
#include <string>
#include "bigunsigned.hpp"
#include "fpelement.hpp"
#include "f2melement.hpp"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string type;
    if (!(std::cin >> type)) {
        std::cerr << "No input\n";
        return 1;
    }

    if (type == "BIG") {
        // BIG <base> <value>
        int base;
        std::string valStr;
        if (!(std::cin >> base >> valStr)) {
            std::cerr << "Invalid BIG input\n";
            return 1;
        }

        BigUnsigned n;
        if (base == 10) {
            n = BigUnsigned::fromBase10(valStr);
        } else if (base == 16) {
            n = BigUnsigned::fromBase16(valStr);
        } else if (base == 64) {
            n = BigUnsigned::fromBase64(valStr);
        } else {
            std::cerr << "Unsupported base for BIG: " << base << "\n";
            return 1;
        }

        std::cout << "BIG " << base << " " << n.toBase16() << "\n";
    }
    else if (type == "FP") {
        // FP <base> <value> <modulus>
        int base;
        std::string valStr, modStr;
        if (!(std::cin >> base >> valStr >> modStr)) {
            std::cerr << "Invalid FP input\n";
            return 1;
        }

        BaseE b;
        if (base == 10)      b = BaseE::BASE_10;
        else if (base == 16) b = BaseE::BASE_16;
        else if (base == 64) b = BaseE::BASE_64;
        else {
            std::cerr << "Unsupported base for FP: " << base << "\n";
            return 1;
        }

        FpElement el(b, valStr, modStr);

        BigUnsigned m = el.getMod();
        BigUnsigned v = el.getVal();

        std::cout << "FP " << m.toBase16() << " " << v.toBase16() << "\n";
    }
    else if (type == "F2M") {
        // F2M <bits> <mod_bits>
        std::string bits, irrBits;
        if (!(std::cin >> bits >> irrBits)) {
            std::cerr << "Invalid F2M input\n";
            return 1;
        }

        F2mElement el(bits, irrBits);

        std::cout << "F2M "
                  << el.modulusToBitString() << " "
                  << el.toBitString() << "\n";
    }
    else {
        std::cerr << "Unknown type: " << type << "\n";
        return 1;
    }

    return 0;
}
