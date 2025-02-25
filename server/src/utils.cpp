#include "utils.hpp"
#include <optional>
#include <string>
#include <vector>

auto cookie::get_val(const std::string &key) const -> std::vector<std::string> {

    std::vector<std::string> data;

    for (const auto &[f, s] : key_val) {

        if (f == key) data.push_back(s);
    }

    return data;
}

auto cookie::is_setting(const std::string &key) const -> bool {

    for (const auto &s : settings) {

        if (s == key) return true;
    }

    return false;
}

auto cookie::get_idsession() const -> std::optional<std::string> {

    auto ids = key_val.find("sessionid");

    if (ids != key_val.end()) { return ids->second; }

    return std::nullopt;
}

auto cookie::get_all_key_val() const
    -> std::unordered_multimap<std::string, std::string> {
    return key_val;
}

auto cookie::get_all_settings() const -> std::vector<std::string> {
    return settings;
}