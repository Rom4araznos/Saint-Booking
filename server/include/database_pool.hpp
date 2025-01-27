#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include "pqxx/pqxx"
#include "structs.hpp"
#include <iostream>

class connection_pool {

    public:
        auto config(const psql_config_t& cfg) -> void;

        auto get_db_url() const -> std::string;

        auto acquire() -> std::shared_ptr<pqxx::connection>;

        auto try_acquire() -> std::shared_ptr<pqxx::connection>;

    protected:
        struct con_info_t {

                pqxx::connection c;
                std::shared_ptr<std::atomic<bool>> acquired =
                    std::make_shared<std::atomic<bool>>(false);
        };

        std::string _url;
        std::vector<con_info_t> _connections;
        std::mutex _mtx;
        std::atomic<bool> status;
        std::uint16_t _max_connections = 20;
};