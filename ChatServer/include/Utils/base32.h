#ifndef BASE32_H
#define BASE32_H
#include <string>
#include <vector>
#include <stdexcept>
#include <array>

namespace base32 {

    const std::string CROCKFORD_ALPHABET = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
    const std::array<int8_t, 256> CROCKFORD_DECODE_TABLE = [] {
        std::array<int8_t, 256> table{};
        table.fill(-1);
        for (size_t i = 0; i < CROCKFORD_ALPHABET.size(); ++i) {
            table[static_cast<uint8_t>(CROCKFORD_ALPHABET[i])] = i;
        }
        // allow lowercase too
        for (char c = 'a'; c <= 'z'; ++c) {
            char uc = static_cast<char>(std::toupper(c));
            int8_t index = table[static_cast<uint8_t>(uc)];
            if (index >= 0) table[static_cast<uint8_t>(c)] = index;
        }
        return table;
        }();

    static std::string encode(const std::array<uint8_t, 16>& bytes) {
        std::string result;
        result.reserve(26);

        uint64_t hi = 0, lo = 0;
        for (int i = 0; i < 8; ++i) hi = (hi << 8) | bytes[i];
        for (int i = 8; i < 16; ++i) lo = (lo << 8) | bytes[i];

        for (int i = 0; i < 10; ++i) {
            result.insert(result.begin(), CROCKFORD_ALPHABET[lo & 0x1F]);
            lo >>= 5;
        }
        lo |= (hi & 0x7) << 61;
        hi >>= 3;

        for (int i = 0; i < 16; ++i) {
            result.insert(result.begin(), CROCKFORD_ALPHABET[hi & 0x1F]);
            hi >>= 5;
        }

        return result;
    }

    static std::array<uint8_t, 16> decode(const std::string& base32_str) {
        if (base32_str.size() != 26) {
            throw std::invalid_argument("ULID string must be 26 characters");
        }

        uint64_t hi = 0, lo = 0;

        for (int i = 0; i < 16; ++i) {
            int8_t val = CROCKFORD_DECODE_TABLE[static_cast<uint8_t>(base32_str[i])];
            if (val < 0) throw std::invalid_argument("Invalid character in ULID");
            hi = (hi << 5) | val;
        }

        for (int i = 16; i < 26; ++i) {
            int8_t val = CROCKFORD_DECODE_TABLE[static_cast<uint8_t>(base32_str[i])];
            if (val < 0) throw std::invalid_argument("Invalid character in ULID");
            lo = (lo << 5) | val;
        }

        lo &= 0xFFFFFFFFFFFFFFFFULL;

        std::array<uint8_t, 16> bytes{};
        for (int i = 7; i >= 0; --i) {
            bytes[i] = static_cast<uint8_t>(hi & 0xFF);
            hi >>= 8;
        }
        for (int i = 15; i >= 8; --i) {
            bytes[i] = static_cast<uint8_t>(lo & 0xFF);
            lo >>= 8;
        }

        return bytes;
    }

} // namespace ulid


#endif // !BASE32_H

