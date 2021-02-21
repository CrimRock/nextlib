#ifndef NEXTLIB_BASE64_HPP
#define NEXTLIB_BASE64_HPP

#include <cstdint>
#include <string>

namespace Next
{
    namespace Encoding
    {
        static const std::string Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const char Base64EncodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        char* EncodeBase64Triplet(uint8_t* data, const int& length = 3) {
            char* ret = new char[5];
            ret[4] = '\0';

            long value = 0;

            if (length >= 1) value += data[0] << 16;
            if (length >= 2) value += data[1] << 8;
            if (length >= 3) value += data[2];

            long mask;
            for (int i = 0; i < 4; i++) {
                int bitshift = ((3 - i) * 6);
                mask = 63;
                mask <<= bitshift;
                mask &= value;
                mask >>= bitshift;
                ret[i] = Base64EncodingTable[mask];
            }

            if (length <= 2) ret[3] = '=';
            if (length <= 1) ret[2] = '=';

            return ret;
        }

        std::string EncodeBase64(std::string data) {
            std::string ret;

            typedef uint8_t byte;
            byte tBuffer[3];
            for (int i = 0; i < data.length();) {
                int j = 0;
                if (i < data.length()) {tBuffer[0] = data.at(i++); j++;}
                if (i < data.length()) {tBuffer[1] = data.at(i++); j++;}
                if (i < data.length()) {tBuffer[2] = data.at(i++); j++;}
                ret += EncodeBase64Triplet(tBuffer, j);
            }

            return ret;
        }

        static inline bool IsBase64(unsigned char c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }

        std::string DecodeBase64(std::string const& data) {
            size_t in_len = data.size();
            size_t i = 0;
            size_t j = 0;
            int in_ = 0;
            unsigned char char_array_4[4], char_array_3[3];
            std::string ret;

            while (in_len-- && (data[in_] != '=') && IsBase64(data[in_])) {
                char_array_4[i++] = data[in_]; in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = static_cast<unsigned char>(Base64Chars.find(char_array_4[i]));
                    
                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; i < 3; i++)
                        ret += char_array_3[i];
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j < 4; j++)
                    char_array_4[j] = 0;
                
                for (j = 0; j < 4; j++)
                    char_array_4[j] = static_cast<unsigned char>(Base64Chars.find(char_array_4[j]));

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; j < i - 1; j++) ret += char_array_3[j];
            }

            return ret;
        }
    }
}

#endif // NEXTLIB_BASE64_HPP