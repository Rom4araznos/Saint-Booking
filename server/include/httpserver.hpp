#pragma once
#define CROW_ENABLE_SSL

#include <memory>
#include "routes.hpp"
#include "crow/app.h"

class httpserver {

    public:
        httpserver(std::shared_ptr<server_routes> &routes) : _routes(routes) {};

        auto set_up_config() -> void;

        auto routes() -> void;

        auto run_server() -> void;

    protected:
        crow::SimpleApp _app;
        std::shared_ptr<server_routes> _routes;
};