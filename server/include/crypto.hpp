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

    auto sha1(std::string_view pass) -> std::string;
    auto sha256(std::string_view pass) -> std::string;
    auto sha512(std::string_view pass) -> std::string;

    const std::string pepper = "9ad";

} // namespace crypto