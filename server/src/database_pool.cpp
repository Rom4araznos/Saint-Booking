#include "database_pool.hpp"

auto connection_pool::config(const psql_config_t& cfg) -> void {

    _url = std::format("postgresql://{}:{}@{}:{}/{}", cfg.username,
                       cfg.password, cfg.host, cfg.port, cfg.db_name);
};

auto connection_pool::get_db_url() const -> std::string {

    return _url;
}

auto connection_pool::acquire() -> std::shared_ptr<pqxx::connection> {

    for (;;) {

        auto ptr = this->try_acquire();

        if (ptr) return ptr;

        status.wait(true);
    }
}

auto connection_pool::try_acquire() -> std::shared_ptr<pqxx::connection> {

    std::lock_guard lock(_mtx);

    for (auto& info : _connections) {

        if (!info.acquired->exchange(true)) {

            return std::shared_ptr<pqxx::connection>(
                &info.c, [acquired = info.acquired, this](pqxx::connection* c) {
                    acquired->store(false);
                    status.store(true);
                    status.notify_one();
                });
        }
    }

    status.store(false);

    if (_connections.size() <= _max_connections) {

        con_info_t info = {

            .c = pqxx::connection(_url)};

        info.acquired->store(true);

        _connections.push_back(std::move(info));

        auto r = std::shared_ptr<pqxx::connection>(
            &_connections.back().c, [acquired = _connections.back().acquired,
                                     this](pqxx::connection* c) {
                acquired->store(false);
                status.store(true);
                status.notify_one();
            });

        return r;
    }

    return nullptr;
}