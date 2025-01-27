#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "crow/json.h"
#include "pqxx/pqxx"
#include "database_pool.hpp"
#include "structs.hpp"

class database {

    public:
        database(std::shared_ptr<connection_pool> &con_pool,
                 std::unordered_map<order_type, std::string> &u_map);

        auto get_sql_from_file(std::string_view path_to)
            -> std::optional<std::string>;
        // auto get_str_opt(const std::variant<std::string, std::nullptr_t>
        // &var)
        //     -> std::optional<std::string>;
        // auto make_opt_vec(
        //     const std::vector<std::variant<std::uint16_t, std::nullptr_t>>
        //         &params_vec) -> std::vector<std::optional<std::uint16_t>>;
        auto sql_bool_array(const std::vector<std::optional<std::uint16_t>>
                                &opt_vec) -> std::string;
        auto hotels_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto hotels_search_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto discounted_hotels_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto discounted_hotels_search_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto places_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto places_search_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto particular_hotel_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;
        auto particular_place_exec(const request_params_t &params)
            -> std::optional<crow::json::wvalue>;

    protected:
        std::shared_ptr<connection_pool> pool;
        std::unordered_map<order_type, std::string> types_of_orders;
};
