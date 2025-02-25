#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class order_type : std::uint16_t {

    t_order_user = 0,
    t_order_star = 1,
    t_order_both = 2,

    t_order_price = 3

};

struct timeframe_t {

        std::optional<std::uint16_t> check_in;
        std::optional<std::uint16_t> cheeck_out;
};

struct request_params_t {

        std::string order;

        timeframe_t timeframe;

        order_type order_by;

        std::uint16_t id;
        std::uint16_t offset;
        std::uint16_t limit;

        std::optional<std::string> num_for_sort;

        std::optional<std::string> person;

        std::vector<std::optional<std::uint16_t>> room_params;

        bool is_reserved;

        std::optional<std::string> country;
        std::optional<std::string> city;
};

struct psql_config_t {

        std::string host;
        std::string db_name;
        std::string username;
        std::string password;
        std::int32_t port;
};