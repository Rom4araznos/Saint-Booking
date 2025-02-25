#include "envloader.hpp"
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

std::unordered_map<std::string, std::string> env_loader::e_val;

env_loader::env_loader(const std::string &path_to_file) {

    std::ifstream file(path_to_file);

    if (!file) throw std::runtime_error("Error: Something with .env file");

    std::stringstream buf;
    buf << file.rdbuf();

    pars(buf.str() + '\n');
}

auto env_loader::pars(std::string_view data) const -> void {

    while (true) {

        auto f1 = data.find_first_not_of(" \n\r\t");

        if (f1 == data.npos) break;

        data.remove_prefix(f1);

        auto f2 = data.find('=');

        auto key = data.substr(0, f2);

        if (key.find(' ') != key.npos)
            throw std::logic_error("Invalid key writing");

        data.remove_prefix(key.length() + 1);

        std::string_view val;

        if (data[0] == '\"') {

            data.remove_prefix(1);

            auto e = data.find("\"\n");

            if (data[e - 1] == '\\')
                throw std::logic_error("Invalid val writing");

            else if (e == data.npos) {
                val = data.substr(0);
                e_val[std::string(key)] = std::string(val);
                data.remove_prefix(val.length() + 1);
                break;
            }

            auto val = data.substr(0, e);

            e_val[std::string(key)] = std::string(val);

            data.remove_prefix(val.length() + 1);
        }

        else {

            auto e = data.find_first_of(" \n");

            if (e == data.npos) {
                val = data.substr(0);

                e_val[std::string(key)] = std::string(val);

                data.remove_prefix(val.length() + 1);
                break;
            }

            val = data.substr(0, e);

            e_val[std::string(key)] = std::string(val);

            data.remove_prefix(val.length() + 1);
        }
    }
}

auto env_loader::get_v() const -> std::unordered_map<std::string, std::string> {

    return e_val;
}