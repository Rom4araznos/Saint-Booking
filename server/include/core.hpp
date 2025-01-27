#pragma once
#include "database.hpp"
#include "database_pool.hpp"
#include "httpserver.hpp"
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>
#include "structs.hpp"

class booking {

    public:
        booking();
        auto run() const -> void;

    protected:
        std::shared_ptr<connection_pool> _database_pool;
        std::shared_ptr<database> _booking_db;
        std::shared_ptr<server_routes> _server_routes;
        std::shared_ptr<httpserver> _httpserver;
};