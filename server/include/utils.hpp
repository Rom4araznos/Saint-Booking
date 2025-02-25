#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
class cookie {

    public:
        static auto
            pars_header(std::string_view data) -> std::optional<cookie> {

            if (data.empty()) return std::nullopt;

            cookie p;

            while (true) {

                auto f1 = data.find_first_not_of(" \n\r");

                if (f1 == data.npos || data.size() == data.npos) break;

                data.remove_prefix(f1);

                auto f = data.find(';');

                auto v_area = data.substr(0, f);

                if (v_area.find_first_of(",=") != v_area.npos) {

                    if (v_area.find('=') != v_area.npos) {

                        auto f2 = v_area.find_first_of(" =");

                        auto key = std::string(v_area.substr(0, f2));

                        v_area.remove_prefix(key.length() + 1);

                        auto f3 = v_area.find_first_not_of(" =");

                        auto val = v_area.substr(f3);

                        if (val.find(',') != val.npos) {

                            auto c_val = val;

                            while (true) {
                                auto sign1 = c_val.find_first_not_of(" ,");

                                if (sign1 == c_val.npos) { break; }

                                c_val.remove_prefix(sign1);

                                auto sign2 = c_val.find_first_of(" ,");

                                if (sign2 == c_val.npos) {
                                    p.key_val.insert({key, std::string(c_val)});
                                    break;
                                };

                                auto inn_val = c_val.substr(0, sign2);

                                p.key_val.insert({key, std::string(inn_val)});

                                c_val.remove_prefix(inn_val.length() + 1);
                            }

                            data.remove_prefix(key.length() + val.length() + 2);
                        }

                        else {

                            p.key_val.insert({key, std::string(val)});

                            data.remove_prefix(key.length() + val.length() + 2);
                        }
                    }

                    else if (v_area.find(',')) {

                        auto f2 = v_area.find(',');

                        auto value = v_area.substr(0, f2);

                        p.settings.push_back(std::string(value));

                        data.remove_prefix(value.length() + 1);
                    }
                }

                else {

                    p.settings.push_back(std::string(v_area));

                    data.remove_prefix(v_area.length() + 1);
                }
            }

            return p;
        }

        auto get_val(const std::string &key) const -> std::vector<std::string>;

        auto get_idsession() const -> std::optional<std::string>;

        auto is_setting(const std::string &key) const -> bool;

        auto get_all_key_val() const
            -> std::unordered_multimap<std::string, std::string>;

        auto get_all_settings() const -> std::vector<std::string>;

    private:
        std::unordered_multimap<std::string, std::string> key_val;
        std::vector<std::string> settings;
};