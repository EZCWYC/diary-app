#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace diary {

constexpr int kSaltSize = 16;
constexpr int kIvSize = 16;
constexpr int kKeySize = 32;
constexpr int kPbkdf2Iterations = 100000;

struct EncryptedData {
    std::vector<uint8_t> salt;
    std::vector<uint8_t> iv;
    std::vector<uint8_t> ciphertext;
};

std::vector<uint8_t> deriveKey(const std::string& password, const std::vector<uint8_t>& salt);

EncryptedData encrypt(const std::string& plaintext, const std::string& password);

std::string decrypt(const std::vector<uint8_t>& ciphertext,
                    const std::vector<uint8_t>& iv,
                    const std::vector<uint8_t>& salt,
                    const std::string& password);

std::string decryptFromBlob(const std::vector<uint8_t>& blob, const std::string& password);

std::vector<uint8_t> encryptToBlob(const std::string& plaintext, const std::string& password);

std::vector<uint8_t> generateRandomBytes(int count);

} // namespace diary
