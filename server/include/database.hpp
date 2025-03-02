#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "crow/http_response.h"
#include "crow/json.h"
#include "pqxx/pqxx"
#include "database_pool.hpp"
#include "structs.hpp"

class database {

    public:
        database(std::shared_ptr<connection_pool> &con_pool,
                 std::unordered_map<order_type, std::string> &settings);

        auto get_sql_from_file(const std::string &path_to) const -> std::string;
        auto sql_bool_array(const std::vector<std::optional<std::uint16_t>>
                                &opt_vec) const -> std::string;
        auto hotels_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto hotels_search_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto discounted_hotels_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto discounted_hotels_search_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto places_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto places_search_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto particular_hotel_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;
        auto particular_place_exec(const request_params_t &params) const
            -> std::optional<crow::json::wvalue>;

        auto get_id_by_token(const std::string &token) const
            -> std::optional<std::string>;
        auto user_reg_exec(const std::string &email,
                           const std::string &pass) const -> crow::response;
        auto user_log_exec(const std::string &email, const std::string &pass)
            const -> std::optional<crow::response>;
        auto pers_id_with_full_data(const std::string &token) const
            -> std::optional<std::string>;
        auto check_expiration(const std::string &token) const -> bool;
        auto res_exec(const std::string &p_id,
                      const crow::json::rvalue &json) const -> crow::response;
        auto res_log_exec(const std::string &id_pers,
                          const crow::json::rvalue &json) const
            -> crow::response;
        auto full_res_exec(const crow::json::rvalue &json) const
            -> std::optional<crow::response>;


    protected:
        std::shared_ptr<connection_pool> pool;
        std::unordered_map<order_type, std::string> types_of_orders;
};
