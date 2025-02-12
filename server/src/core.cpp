#include "core.hpp"
#include <asio/detail/throw_exception.hpp>
#include <cstdlib>
#include <stdexcept>
#include <string>

booking::booking() {

    char* h = std::getenv("DB_HOST");
    char* n = std::getenv("DB_NAME");
    char* un = std::getenv("DB_USERNAME");
    char* pass = std::getenv("DB_PASSWORD");
    char* port = std::getenv("DB_PORT");


    if (h == nullptr || n == nullptr || un == nullptr || pass == nullptr ||
        port == nullptr)
        throw std::logic_error("Error: Invalid connection data");


    psql_config_t config{

        .host = h,
        .db_name = n,
        .username = un,
        .password = pass,
        .port = std::stoi(port)

    };

    _database_pool = std::make_shared<connection_pool>();

    _database_pool->config(config);

    std::unordered_map<order_type, std::string> order_type_map;

    order_type_map[order_type::t_order_star] = "stars";
    order_type_map[order_type::t_order_user] = "user_rating";
    order_type_map[order_type::t_order_both] = "((stars + user_rating) / 2)";
    order_type_map[order_type::t_order_price] = "price";

    _booking_db = std::make_shared<database>(_database_pool, order_type_map);

    _server_routes = std::make_shared<server_routes>(_booking_db);

    _httpserver = std::make_shared<httpserver>(_server_routes);

    _httpserver->set_up_config();
    _httpserver->routes();
}

auto booking::run() const -> void {

    _httpserver->run_server();
}