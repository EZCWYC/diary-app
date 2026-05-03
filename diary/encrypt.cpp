#include "diary/encrypt.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include <cstring>
#include <stdexcept>

namespace diary {

std::vector<uint8_t> generateRandomBytes(int count) {
    std::vector<uint8_t> buffer(static_cast<size_t>(count));
    if (RAND_bytes(buffer.data(), count) != 1) {
        throw std::runtime_error("随机数生成失败");
    }
    return buffer;
}

void secureZeroMemory(void* ptr, size_t size) {
    if (ptr != nullptr && size > 0) {
        OPENSSL_cleanse(ptr, size);
    }
}

std::vector<uint8_t> deriveKey(const std::string& password,
                               const std::vector<uint8_t>& salt,
                               int keyLen) {
    std::vector<uint8_t> key(static_cast<size_t>(keyLen));
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        salt.data(),
        static_cast<int>(salt.size()),
        kPbkdf2Iterations,
        EVP_sha256(),
        keyLen,
        key.data());
    if (result != 1) {
        throw std::runtime_error("PBKDF2 密钥派生失败");
    }
    return key;
}

DerivedKeys deriveKeys(const std::string& password,
                       const std::vector<uint8_t>& salt) {
    constexpr int totalKeyLen = kKeySize * 2;
    std::vector<uint8_t> material = deriveKey(password, salt, totalKeyLen);

    DerivedKeys keys;
    keys.aesKey.assign(material.begin(), material.begin() + kKeySize);
    keys.hmacKey.assign(material.begin() + kKeySize, material.end());

    secureZeroMemory(material.data(), material.size());
    return keys;
}

EncryptedData encrypt(const std::string& plaintext, const std::string& password) {
    std::vector<uint8_t> salt = generateRandomBytes(kSaltSize);
    std::vector<uint8_t> iv = generateRandomBytes(kIvSize);
    std::vector<uint8_t> key = deriveKey(password, salt, kKeySize);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("创建加密上下文失败");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("初始化 AES-256-CBC 加密失败");
    }

    std::vector<uint8_t> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int totalLen = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          reinterpret_cast<const uint8_t*>(plaintext.data()),
                          static_cast<int>(plaintext.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("AES 加密更新失败");
    }
    totalLen += len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + totalLen, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("AES 加密收尾失败");
    }
    totalLen += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(static_cast<size_t>(totalLen));

    secureZeroMemory(key.data(), key.size());
    return {salt, iv, ciphertext};
}

std::string decrypt(const std::vector<uint8_t>& ciphertext,
                    const std::vector<uint8_t>& iv,
                    const std::vector<uint8_t>& salt,
                    const std::string& password) {
    std::vector<uint8_t> key = deriveKey(password, salt, kKeySize);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("创建解密上下文失败");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("初始化 AES-256-CBC 解密失败");
    }

    std::vector<uint8_t> plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int totalLen = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                          ciphertext.data(),
                          static_cast<int>(ciphertext.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("AES 解密更新失败");
    }
    totalLen += len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + totalLen, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        secureZeroMemory(key.data(), key.size());
        throw std::runtime_error("AES 解密失败：密码错误或数据损坏");
    }
    totalLen += len;

    EVP_CIPHER_CTX_free(ctx);
    secureZeroMemory(key.data(), key.size());

    std::string result(reinterpret_cast<char*>(plaintext.data()), static_cast<size_t>(totalLen));
    secureZeroMemory(plaintext.data(), plaintext.size());
    return result;
}

std::vector<uint8_t> encryptToBlob(const std::string& plaintext, const std::string& password) {
    EncryptedData data = encrypt(plaintext, password);

    std::vector<uint8_t> payload;
    payload.reserve(data.salt.size() + data.iv.size() + data.ciphertext.size());

    payload.insert(payload.end(), data.salt.begin(), data.salt.end());
    payload.insert(payload.end(), data.iv.begin(), data.iv.end());
    payload.insert(payload.end(), data.ciphertext.begin(), data.ciphertext.end());

    DerivedKeys keys = deriveKeys(password, data.salt);

    std::vector<uint8_t> hmac(kHmacSize);
    unsigned int hmacLen = 0;
    HMAC(EVP_sha256(),
         keys.hmacKey.data(), static_cast<int>(keys.hmacKey.size()),
         payload.data(), payload.size(),
         hmac.data(), &hmacLen);

    secureZeroMemory(keys.aesKey.data(), keys.aesKey.size());
    secureZeroMemory(keys.hmacKey.data(), keys.hmacKey.size());

    hmac.resize(hmacLen);
    payload.insert(payload.end(), hmac.begin(), hmac.end());

    return payload;
}

std::string decryptFromBlob(const std::vector<uint8_t>& blob, const std::string& password) {
    if (blob.size() < static_cast<size_t>(kSaltSize + kIvSize + kHmacSize + 1)) {
        throw std::runtime_error("密文数据格式错误：数据太短");
    }

    size_t payloadLen = blob.size() - kHmacSize;

    std::vector<uint8_t> salt(blob.begin(), blob.begin() + kSaltSize);
    std::vector<uint8_t> iv(blob.begin() + kSaltSize, blob.begin() + kSaltSize + kIvSize);
    std::vector<uint8_t> ciphertext(blob.begin() + kSaltSize + kIvSize, blob.begin() + payloadLen);

    std::vector<uint8_t> storedHmac(blob.begin() + payloadLen, blob.end());

    DerivedKeys keys = deriveKeys(password, salt);

    std::vector<uint8_t> computedHmac(kHmacSize);
    unsigned int hmacLen = 0;
    HMAC(EVP_sha256(),
         keys.hmacKey.data(), static_cast<int>(keys.hmacKey.size()),
         blob.data(), payloadLen,
         computedHmac.data(), &hmacLen);

    secureZeroMemory(keys.hmacKey.data(), keys.hmacKey.size());

    bool hmacValid = (hmacLen == storedHmac.size());
    if (hmacValid) {
        int result = CRYPTO_memcmp(computedHmac.data(), storedHmac.data(), hmacLen);
        hmacValid = (result == 0);
    }

    if (!hmacValid) {
        secureZeroMemory(keys.aesKey.data(), keys.aesKey.size());
        throw std::runtime_error("HMAC 校验失败：密码错误或数据被篡改");
    }

    secureZeroMemory(keys.aesKey.data(), keys.aesKey.size());
    return decrypt(ciphertext, iv, salt, password);
}

} // namespace diary
