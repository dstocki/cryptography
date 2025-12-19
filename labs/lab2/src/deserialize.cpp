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
        int base;
        std::string hexStr;
        if (!(std::cin >> base >> hexStr)) {
            std::cerr << "Invalid BIG canonical input\n";
            return 1;
        }

        BigUnsigned n = BigUnsigned::fromBase16(hexStr);

        std::string out = "";
        if (base == 10) {
            out = n.toBase10();
        } else if (base == 16) {
            out = n.toBase16();
        } else if (base == 64) {
            out = n.toBase64();
        } else {
            std::cerr << "Unsupported base for BIG: " << base << "\n";
            return 1;
        }

        std::cout << "BIG " << base << " " << out << "\n";
    }
    else if (type == "FP") {
        std::string modHex, valHex;
        if (!(std::cin >> modHex >> valHex)) {
            std::cerr << "Invalid FP canonical input\n";
            return 1;
        }

        FpElement el(BaseE::BASE_16, valHex, modHex);

        BigUnsigned m = el.getMod();
        BigUnsigned v = el.getVal();

        std::cout << "FP " << m.toBase16() << " " << v.toBase16() << "\n";
    }
    else if (type == "F2M") {
        std::string modBits, valBits;
        if (!(std::cin >> modBits >> valBits)) {
            std::cerr << "Invalid F2M canonical input\n";
            return 1;
        }

        F2mElement el(valBits, modBits);

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
