#pragma once
#include <iostream>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>
#include <sys/stat.h>

namespace crypto {

    auto rand_bytes(std::uint32_t size) -> std::optional<std::string>;
    auto hash_to_hex(const std::string &data) -> std::string;
    auto create_hash_evp(const EVP_MD *mod,
                         const std::string &data) -> std::optional<std::string>;

    const std::string alph = "0123456789abcdef";
    const std::string pepper = "9ad";

} // namespace crypto