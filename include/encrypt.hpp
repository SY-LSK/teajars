#ifndef ENCRYPT_HPP
#define ENCRYPT_HPP

#include <string>
#include <vector>

class Encrypt{
private:
    static const std::string base64_chars;

public:
    //XOR加密/解密
    static std::string xor_encrypt_decrypt(const std::string& data, const std::string& key) {
        if (key.empty()) return data;
        
        std::string result = data;
        for (size_t i = 0; i < data.length(); i++) {
            result[i] = data[i] ^ key[i % key.length()];
        }
        return result;
    }
    
    //Base64编码
    static std::string base64_encode(const std::string& input) {
        std::string output;
        int val = 0, valb = -6;

        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                output.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (output.size() % 4) output.push_back('=');
        return output;
    }
    
    //Base64解码
    static std::string base64_decode(const std::string& input) {
        std::string output;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                output.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return output;
    }
    
    //XOR加密再Base64编码
    static std::string encrypt(const std::string& data, const std::string& key) {
        std::string xorResult = xor_encrypt_decrypt(data, key);
        return base64_encode(xorResult);
    }
    
    //Base64解码再XOR解密
    static std::string decrypt(const std::string& encryptedData, const std::string& key) {
        std::string decoded = base64_decode(encryptedData);
        return xor_encrypt_decrypt(decoded, key);
    }
};

// 定义静态成员变量
const std::string Encrypt::base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#endif // ENCRYPT_HPP