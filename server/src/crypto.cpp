#include "crypto.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

auto crypto::rand_bytes(std::uint32_t size) -> std::optional<std::string> {

    std::string buf;

    buf.resize(size);

    auto rand = RAND_bytes(reinterpret_cast<std::uint8_t *>(buf.data()), size);

    if (rand != 1) return std::nullopt;

    return buf;
}

auto crypto::create_hash_evp(const EVP_MD *mod, const std::string &data)
    -> std::optional<std::string> {

    std::string hash_bin_pass;

    EVP_MD_CTX *cont = EVP_MD_CTX_new();

    if (!cont) return std::nullopt;
    if (!EVP_DigestInit_ex(cont, mod, NULL)) return std::nullopt;
    if (!EVP_DigestUpdate(cont, data.data(), data.size())) return std::nullopt;

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int h_length = 0;

    if (!EVP_DigestFinal(cont, hash, &h_length)) return std::nullopt;

    hash_bin_pass.assign(hash, hash + h_length);

    return hash_bin_pass;
};

auto crypto::hash_to_hex(const std::string &data) -> std::string {

    std::stringstream ss;

    for (unsigned char c : data) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(c);
    }

    return ss.str();
}