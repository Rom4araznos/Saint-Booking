#include "httpserver.hpp"
#include "crow/app.h"
#include "crow/common.h"
#include "crow/http_request.h"
#include "crow/json.h"
#include "database.hpp"
#include "routes.hpp"


auto httpserver::run_server() -> void {

    _app.port(29172).multithreaded().run();
};

auto httpserver::set_up_config() -> void {

    try {

        asio::ssl::context ctx(asio::ssl::context::tls_server);

        ctx.set_options(

            asio::ssl::context::no_sslv2 | asio::ssl::context::no_sslv3 |
            asio::ssl::context::no_tlsv1 | asio::ssl::context::no_tlsv1_1 |
            asio::ssl::context::no_tlsv1_2);

        ctx.use_certificate_chain_file("server.pem");
        ctx.use_private_key_file("server.key", asio::ssl::context::pem);

        ctx.set_verify_mode(asio::ssl::verify_none);

        _app.ssl(std::move(ctx));

    } catch (std::exception &exc) {

        std::cerr << "The SSL connection could not be established: "
                  << exc.what() << std::endl;
    }
}

auto httpserver::routes() -> void {

    CROW_ROUTE(_app, "/")
    ([]() {
        return "hello";
    });

    CROW_ROUTE(_app, "/api/hotels")
    ([this](const crow::request &req) {
        return _routes->hotels(req.url_params);
    });

    CROW_ROUTE(_app, "/api/hotels/search")
    ([this](const crow::request &req) {
        return _routes->hotels_search(req.url_params);
    });

    CROW_ROUTE(_app, "/api/places")
    ([this](const crow::request &req) {
        crow::query_string params = req.url_params;

        return _routes->places(req.url_params);
    });

    CROW_ROUTE(_app, "/api/places/search")
    ([this](const crow::request &req) {
        crow::query_string params = req.url_params;

        return _routes->places_search(req.url_params);
    });

    CROW_ROUTE(_app, "/api/hotels/discounted")
    ([this](const crow::request &req) {
        return _routes->discounted_hotels(req.url_params);
    });

    CROW_ROUTE(_app, "/api/hotels/discounted/search")
    ([this](const crow::request &req) {
        return _routes->discounted_hotels_search(req.url_params);
    });

    CROW_ROUTE(_app, "/api/hotel/info")
    ([this](const crow::request &req) {
        return _routes->particular_info_hotels(req.url_params);
    });

    CROW_ROUTE(_app, "/api/place/info")
    ([this](const crow::request &req) {
        return _routes->particular_info_places(req.url_params);
    });

    CROW_ROUTE(_app, "/api/join")
        .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
            return _routes->user_reg(req);
        });

    CROW_ROUTE(_app, "/api/login")
        .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
            return _routes->user_log(req);
        });

    CROW_ROUTE(_app, "/api/order/reserve")
        .methods(crow::HTTPMethod::POST)([this](const crow::request &req) {
            return _routes->apartment_res(req);
        });
}