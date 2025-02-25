#include "crypto.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
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

auto crypto::sha1(std::string_view pass) -> std::string {

    std::string buf;
    buf.resize(20);

    auto bytes = reinterpret_cast<const unsigned char *>(pass.data());
    auto out = reinterpret_cast<unsigned char *>(buf.data());

    SHA1(bytes, pass.size(), out);

    return buf;
}

auto crypto::sha256(std::string_view pass) -> std::string {

    std::string buf;
    buf.resize(32);

    auto bytes = reinterpret_cast<const unsigned char *>(pass.data());
    auto out = reinterpret_cast<unsigned char *>(buf.data());

    SHA256(bytes, pass.size(), out);

    return buf;
}

auto crypto::sha512(std::string_view pass) -> std::string {

    std::string buf;
    buf.resize(64);

    auto bytes = reinterpret_cast<const unsigned char *>(pass.data());
    auto out = reinterpret_cast<unsigned char *>(buf.data());

    SHA512(bytes, pass.size(), out);

    return buf;
}

auto crypto::hex(const std::string &data) -> std::string {

    std::stringstream ss;

    for (unsigned char c : data) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(c);
    }

    return ss.str();
}