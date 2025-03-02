#include "crow/ci_map.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include "crow/json.h"
#include "crow/query_string.h"
#include <asio/ssl/context.hpp>
#include <asio/ssl/verify_mode.hpp>
#include <cstdlib>
#include <exception>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include "database.hpp"
#include "structs.hpp"
#include "routes.hpp"
#include "openssl/sha.h"
#include "utils.hpp"

auto server_routes::opt_int_queries(const char *param) const
    -> std::optional<std::uint16_t> {

    if (param) return std::uint16_t(std::stoi(param));

    return std::nullopt;
}

auto server_routes::opt_str_queries(const char *param) const
    -> std::optional<std::string> {

    if (param) return std::string(param);

    return std::nullopt;
}

auto server_routes::opt_ll_queries(const char *param) const
    -> std::optional<std::uint64_t> {

    if (param) return std::uint64_t(std::stoll(param));

    return std::nullopt;
}

auto server_routes::query_params_hotels(const crow::query_string &params) const
    -> std::variant<request_params_t, crow::response> {

    request_params_t req_params;

    auto offset = params.get("offset");
    auto limit = params.get("limit");
    auto order = params.get("order");
    auto order_by = params.get("order_by");
    auto person = params.get("person");

    // Добавить время резерв

    if (!offset || !limit || !order || !order_by) {

        return crow::response(400, "Invalid params supplied");
    }

    auto climate_control = params.get("ce_cl");
    auto high_speed_wifi = params.get("hh_sd_wifi");
    auto private_bathroom = params.get("p_bathroom");
    auto tv_add_opt = params.get("tv_opt");
    auto mini_bar = params.get("mini_bar");
    auto in_room_safe = params.get("r_safe");
    auto work_features = params.get("wk_fs");
    auto room_service = params.get("rm_se");
    auto room_cleaning = params.get("rm_cg");
    auto transport_service = params.get("tt_se");
    auto luggage_storage = params.get("le_sge");
    auto full_time_support = params.get("fullt_supp");
    auto fitness_center = params.get("fit_cr");
    auto in_room_workout_equip = params.get("irwe");
    auto spa = params.get("spa");
    auto swimming_pool_cov = params.get("sg_pl_cov");
    auto swimming_pool_unc = params.get("sg_pl_unc");
    auto business_center = params.get("bs_cr");
    auto meeting_rooms = params.get("m_rooms");
    auto restaurant = params.get("rt");
    auto breakfast_options = params.get("bt_opt");
    auto in_room_dining = params.get("ird");
    auto num_for_sort = params.get("sort_num");
    auto children = params.get("children");
    auto pet = params.get("pet");
    auto check_in = params.get("check_in");
    auto check_out = params.get("check_out");
    auto free_cancel = params.get("f_cancel");
    auto country = params.get("cntr");
    auto city = params.get("ct");

    try {

        req_params.offset = std::stoi(offset);
        req_params.limit = std::stoi(limit);
        req_params.order = order;
        req_params.order_by = order_type(std::stoi(order_by));
        req_params.person = opt_str_queries(person);
        req_params.num_for_sort = opt_str_queries(num_for_sort);
        req_params.timeframe.check_in = opt_ll_queries(check_in);
        req_params.timeframe.cheeck_out = opt_ll_queries((check_out));
        req_params.country = opt_str_queries(country);

        req_params.room_params.push_back(opt_int_queries(climate_control));
        req_params.room_params.push_back(opt_int_queries(high_speed_wifi));
        req_params.room_params.push_back(opt_int_queries(private_bathroom));
        req_params.room_params.push_back(opt_int_queries(tv_add_opt));
        req_params.room_params.push_back(opt_int_queries(mini_bar));
        req_params.room_params.push_back(opt_int_queries(in_room_safe));
        req_params.room_params.push_back(opt_int_queries(work_features));
        req_params.room_params.push_back(opt_int_queries(room_service));
        req_params.room_params.push_back(opt_int_queries(room_cleaning));
        req_params.room_params.push_back(opt_int_queries(transport_service));
        req_params.room_params.push_back(opt_int_queries(luggage_storage));
        req_params.room_params.push_back(opt_int_queries(full_time_support));
        req_params.room_params.push_back(opt_int_queries(fitness_center));
        req_params.room_params.push_back(
            opt_int_queries(in_room_workout_equip));
        req_params.room_params.push_back(opt_int_queries(spa));
        req_params.room_params.push_back(opt_int_queries(swimming_pool_cov));
        req_params.room_params.push_back(opt_int_queries(swimming_pool_unc));
        req_params.room_params.push_back(opt_int_queries(business_center));
        req_params.room_params.push_back(opt_int_queries(meeting_rooms));
        req_params.room_params.push_back(opt_int_queries(restaurant));
        req_params.room_params.push_back(opt_int_queries(breakfast_options));
        req_params.room_params.push_back(opt_int_queries(in_room_dining));
        req_params.room_params.push_back(opt_int_queries(children));
        req_params.room_params.push_back(opt_int_queries(pet));
        req_params.room_params.push_back(opt_int_queries(free_cancel));

    } catch (const std::exception &exc) {
        std::cout << "Could not convert query parameter to the number: "
                  << exc.what() << std::endl;

        return crow::response(400, "Expected numeric params - got string");
    }


    return req_params;
}

auto server_routes::query_params_places(const crow::query_string &params) const
    -> std::variant<request_params_t, crow::response> {

    request_params_t req_params;

    auto order = params.get("order");
    auto order_by = params.get("order_by");
    auto offset = params.get("offset");
    auto limit = params.get("limit");

    if (!order || !order_by || !offset || !limit) {

        return crow::response(400, "Invalid params");
    }

    auto num_for_sort = params.get("sort_num");
    auto country = params.get("cntr");
    auto city = params.get("ct");

    try {

        req_params.order = order;
        req_params.order_by = order_type(std::stoi(order_by));
        req_params.offset = std::stoul(offset);
        req_params.limit = std::stoi(limit);
        req_params.num_for_sort = opt_str_queries(num_for_sort);
        req_params.country = opt_str_queries(country);
        req_params.city = opt_str_queries(city);

    } catch (const std::exception &exc) {

        std::cout << "Could not convert query parameter to the number: "
                  << exc.what() << std::endl;

        return crow::response(400, "Expected numeric params - got string");
    }

    return req_params;
}

auto server_routes::query_particular_obj(const crow::query_string &params) const
    -> std::variant<request_params_t, crow::response> {

    request_params_t req_params;

    auto id = params.get("id");

    if (!id) return crow::response(400, "Invalid params");

    try {

        req_params.id = std::stoi(id);

        return req_params;
    } catch (const std::exception &exc) {

        std::cout << "Could not convert query parameter to the number: "
                  << exc.what() << std::endl;

        return crow::response(400, "Expected numeric params - got string");
    }
}

auto server_routes::hotels(const crow::query_string &params) const
    -> crow::response {

    auto r_hotels = query_params_hotels(params);

    if (std::holds_alternative<crow::response>(r_hotels))

        return std::move(std::get<crow::response>(r_hotels));

    auto json = _database->hotels_exec(std::get<request_params_t>(r_hotels));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}

auto server_routes::hotels_search(const crow::query_string &params) const
    -> crow::response {

    auto r_hotels = query_params_hotels(params);

    if (std::holds_alternative<crow::response>(r_hotels))

        return std::move(std::get<crow::response>(r_hotels));

    auto json = _database->hotels_search_exec(
        std::get<request_params_t>(r_hotels));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}

auto server_routes::discounted_hotels(const crow::query_string &params) const
    -> crow::response {

    auto r_discounted_hotel = query_params_hotels(params);

    if (std::holds_alternative<crow::response>(r_discounted_hotel))

        return std::move(std::get<crow::response>(r_discounted_hotel));

    auto json = _database->discounted_hotels_exec(
        std::get<request_params_t>(r_discounted_hotel));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}

auto server_routes::discounted_hotels_search(
    const crow::query_string &params) const -> crow::response {

    auto r_discounted_hotel = query_params_hotels(params);

    if (std::holds_alternative<crow::response>(r_discounted_hotel))

        return std::move(std::get<crow::response>(r_discounted_hotel));

    auto json = _database->discounted_hotels_search_exec(
        std::get<request_params_t>(r_discounted_hotel));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
};

auto server_routes::places(const crow::query_string &params) const
    -> crow::response {

    auto r_places = query_params_places(params);

    if (std::holds_alternative<crow::response>(r_places))

        return std::move(std::get<crow::response>(r_places));

    auto json = _database->places_exec(std::get<request_params_t>(r_places));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}

auto server_routes::places_search(const crow::query_string &params) const
    -> crow::response {

    auto r_places = query_params_places(params);

    if (std::holds_alternative<crow::response>(r_places))

        return std::move(std::get<crow::response>(r_places));

    auto json = _database->places_search_exec(
        std::get<request_params_t>(r_places));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}


auto server_routes::particular_info_hotels(
    const crow::query_string &params) const -> crow::response {

    auto r_particular_hotel = query_particular_obj(params);

    if (std::holds_alternative<crow::response>(r_particular_hotel))
        return std::move(std::get<crow::response>(r_particular_hotel));

    auto json = _database->particular_hotel_exec(
        std::get<request_params_t>(r_particular_hotel));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}

auto server_routes::particular_info_places(
    const crow::query_string &params) const -> crow::response {

    auto r_particular_hotel = query_particular_obj(params);

    if (std::holds_alternative<crow::response>(r_particular_hotel))
        return std::move(std::get<crow::response>(r_particular_hotel));

    auto json = _database->particular_place_exec(
        std::get<request_params_t>(r_particular_hotel));

    if (!json.has_value())
        return crow::response(400, "The database did not return a value.");

    return *json;
}

auto server_routes::user_reg(const crow::request &req) const -> crow::response {

    auto json = crow::json::load(req.body);

    if (!json) return crow::response(400, "The client has not sent the data");

    std::string e = json["email"].s();
    std::string p = json["password"].s();

    if (e.empty() || p.empty())
        return crow::response(400, "The client has sent incorrect data");

    auto resp = _database->user_reg_exec(e, p);

    return resp;
}

auto server_routes::user_log(const crow::request &req) const -> crow::response {

    auto json = crow::json::load(req.body);

    if (!json) return crow::response(400, "The client has not sent the data");

    std::string e = json["email"].s();
    std::string p = json["password"].s();

    if (e.empty() || p.empty())
        return crow::response(400, "The client has sent incorrect data");

    auto resp = _database->user_log_exec(e, p);

    if (!resp.has_value())
        return crow::response(400, "Incorrect authorization data");

    return std::move(*resp);
}

auto server_routes::apartment_res(const crow::request &req) const
    -> crow::response {

    std::string header = req.get_header_value("Cookie");

    auto json = crow::json::load(req.body);

    if (auto p = cookie::pars_header(header)) {

        auto sessionid = p->get_idsession();

        if ((sessionid.has_value())) {

            if (!_database->check_expiration(*sessionid))
                return crow::response(403, "The session time has expired");

            if (!json)
                return crow::response(
                    403, "The client is registered and logged in, "
                         "however have not provided full information");

            auto id_pers_full_data = _database->pers_id_with_full_data(
                *sessionid);

            if (!id_pers_full_data.has_value()) {

                auto id_pers = _database->get_id_by_token(*sessionid);

                return _database->res_log_exec(*id_pers, json);
            };

            return _database->res_exec(*id_pers_full_data, json);
        }
    }

    if (json) { return *_database->full_res_exec(json); }

    return crow::response(403, "The client has not sent any data");
}
