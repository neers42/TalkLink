#ifndef BASE32_HPP
#define BASE32_HPP
#include <string>
#include <vector>
#include <stdexcept>
#include <array>
#include <map>
#include <stdint.h>
#include <sstream>


namespace base32 {

    const std::string CROCKFORD_ALPHABET = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

    const std::array<int8_t, 256> CROCKFORD_DECODE_TABLE = [] {
        std::array<int8_t, 256> table{};
        table.fill(-1);
        for (size_t i = 0; i < CROCKFORD_ALPHABET.size(); ++i) {
            table[static_cast<uint8_t>(CROCKFORD_ALPHABET[i])] = static_cast<int8_t>(i);
        }
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

        uint32_t bit_buffer = 0;
        int bits_in_buffer = 0;
        size_t byte_index = 0;

        while (result.size() < 26) {
            if (bits_in_buffer < 5) {
                if (byte_index < bytes.size()) {
                    bit_buffer = (bit_buffer << 8) | bytes[byte_index++];
                    bits_in_buffer += 8;
                }
                else {
                    // 残りバイトなし → 0 padding
                    bit_buffer <<= (5 - bits_in_buffer);
                    bits_in_buffer = 5;
                }
            }

            int index = (bit_buffer >> (bits_in_buffer - 5)) & 0x1F;
            bits_in_buffer -= 5;
            result.push_back(CROCKFORD_ALPHABET[index]);
        }

        return result;
    }

    static std::array<uint8_t, 16> decode(const std::string& base32_str) {
        if (base32_str.length() != 26)
            throw std::invalid_argument("ULID must be 26 characters");

        std::array<uint8_t, 16> result{};
        uint32_t bit_buffer = 0;
        int bits_in_buffer = 0;
        size_t byte_index = 0;

        for (char c : base32_str) {
            int val = CROCKFORD_DECODE_TABLE[static_cast<uint8_t>(c)];
            if (val < 0) throw std::invalid_argument("Invalid ULID character");

            bit_buffer = (bit_buffer << 5) | val;
            bits_in_buffer += 5;

            while (bits_in_buffer >= 8 && byte_index < 16) {
                bits_in_buffer -= 8;
                result[byte_index++] = static_cast<uint8_t>((bit_buffer >> bits_in_buffer) & 0xFF);
            }
        }

        return result;
    }

} // namespace base32


//
//namespace base32 {
//
//    const std::string CROCKFORD_ALPHABET = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
//    const std::array<int8_t, 256> CROCKFORD_DECODE_TABLE = [] {
//        std::array<int8_t, 256> table{};
//        table.fill(-1);
//        for (size_t i = 0; i < CROCKFORD_ALPHABET.size(); ++i) {
//            table[static_cast<uint8_t>(CROCKFORD_ALPHABET[i])] = i;
//        }
//        // allow lowercase too
//        for (char c = 'a'; c <= 'z'; ++c) {
//            char uc = static_cast<char>(std::toupper(c));
//            int8_t index = table[static_cast<uint8_t>(uc)];
//            if (index >= 0) table[static_cast<uint8_t>(c)] = index;
//        }
//        return table;
//        }();
//
//    static std::string encode(const std::array<uint8_t, 16>& bytes) {
//
//        static constexpr char CROCKFORD_ALPHABET[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
//        std::string result;
//        result.reserve(26);
//
//        int bit_buffer = 0;
//        int bits_in_buffer = 0;
//        size_t byte_index = 0;
//
//        while (result.size() < 26) {
//            if (bits_in_buffer < 5) {
//                if (byte_index < bytes.size()) {
//                    bit_buffer = (bit_buffer << 8) | bytes[byte_index++];
//                    bits_in_buffer += 8;
//                }
//                else {
//                    // もう追加できるバイトがないので、末尾に0を詰める
//                    bit_buffer <<= (5 - bits_in_buffer);
//                    bits_in_buffer = 5;
//                }
//            }
//
//            int index = (bit_buffer >> (bits_in_buffer - 5)) & 0x1F;
//            bits_in_buffer -= 5;
//            result.push_back(CROCKFORD_ALPHABET[index]);
//        }
//
//        return result;
//
//
//        }
//
//    static std::array<uint8_t, 16> decode(const std::string& base32_str) {
//
//        if (base32_str.length() != 26)
//            throw std::invalid_argument("ULID must be 26 characters");
//
//        std::array<uint8_t, 16> result{};
//        int bit_buffer = 0;
//        int bits_in_buffer = 0;
//        size_t byte_index = 0;
//
//        for (char c : base32_str) {
//            int val = base32::CROCKFORD_DECODE_TABLE[static_cast<uint8_t>(c)];
//            if (val < 0) throw std::invalid_argument("Invalid ULID character");
//
//            bit_buffer = (bit_buffer << 5) | val;
//            bits_in_buffer += 5;
//
//            if (bits_in_buffer >= 8) {
//                bits_in_buffer -= 8;
//                result[byte_index++] = static_cast<uint8_t>((bit_buffer >> bits_in_buffer) & 0xFF);
//                if (byte_index >= 16) break;
//            }
//        }
//
//        return result;
//
//    }
//
//} // namespace ulid


#endif // !BASE32_HPP
