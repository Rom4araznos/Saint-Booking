#pragma once

#include "crow/http_response.h"
#include "crow/json.h"
#include "crow/query_string.h"
#include "structs.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include "database.hpp"

class server_routes {

    public:
        server_routes(std::shared_ptr<database> &db) : _database(db) {};

        auto opt_int_queries(const char *param) -> std::optional<std::uint16_t>;

        auto opt_str_queries(const char *param) -> std::optional<std::string>;

        auto opt_ll_queries(const char *param) -> std::optional<std::uint64_t>;

        auto query_params_hotels(const crow::query_string &params)
            -> std::variant<request_params_t, crow::response>;

        auto query_params_places(const crow::query_string &params)
            -> std::variant<request_params_t, crow::response>;

        auto query_particular_obj(const crow::query_string &params)
            -> std::variant<request_params_t, crow::response>;

        auto hotels(const crow::query_string &params) -> crow::response;

        auto hotels_search(const crow::query_string &params) -> crow::response;

        auto places(const crow::query_string &params) -> crow::response;

        auto places_search(const crow::query_string &params) -> crow::response;

        auto discounted_hotels(const crow::query_string &params)
            -> crow::response;

        auto discounted_hotels_search(const crow::query_string &params)
            -> crow::response;

        auto particular_info_hotels(const crow::query_string &params)
            -> crow::response;

        auto particular_info_places(const crow::query_string &params)
            -> crow::response;

        auto user_reg(const crow::json::rvalue &data) -> crow::response;

        auto user_log(const crow::json::rvalue &data) -> crow::response;

    private:
        std::shared_ptr<database> _database;
};