#include "core.hpp"
#include <asio/detail/throw_exception.hpp>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include "envloader.hpp"

booking::booking() {

    env_loader env_file(".env");
    auto val = env_file.get_v();

    std::string h = val["DB_HOST"];
    std::string n = val["DB_NAME"];
    std::string un = val["DB_USERNAME"];
    std::string pass = val["DB_PASSWORD"];
    std::string port = val["DB_PORT"];

    if (h.empty() || n.empty() || un.empty() || pass.empty() || port.empty())
        throw std::runtime_error("Error: Invalid connection data");

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