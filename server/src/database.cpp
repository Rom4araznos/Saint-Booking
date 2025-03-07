#include "database.hpp"
#include "crow/http_response.h"
#include "crow/json.h"
#include "crow/returnable.h"
#include "crypto.hpp"
#include "database_pool.hpp"
#include "structs.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <fstream>
#include <openssl/evp.h>
#include <optional>
#include <pqxx/internal/statement_parameters.hxx>
#include <pqxx/zview.hxx>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <string_view>
#include <vector>

database::database(std::shared_ptr<connection_pool> &con_pool,
                   std::unordered_map<order_type, std::string> &ord_filters)
    : pool(con_pool), types_of_orders(ord_filters) {

    std::vector<std::string> paths_to = {
        "bin_sql/functions/select_hotels_start.sql",
        "bin_sql/functions/select_hotels_search.sql",
        "bin_sql/functions/select_discounted_hotels_start.sql",
        "bin_sql/functions/select_discounted_hotels_search.sql",
        "bin_sql/functions/select_places_start.sql",
        "bin_sql/functions/select_places_search.sql",
        "bin_sql/functions/select_hotel.sql",
        "bin_sql/functions/select_place.sql",
        "bin_sql/functions/check_session_token.sql"};

    auto conn = pool->acquire();

    pqxx::work tx{*conn};

    for (const auto &path_to : paths_to) {

        auto sql_func = get_sql_from_file(path_to);

        if (sql_func.empty())
            std::cout << "Error: " << "Something with sql function's file"
                      << std::endl;

        tx.exec(sql_func);
    }

    tx.commit();
};

auto database::get_sql_from_file(const std::string &path_to) const
    -> std::string {

    std::ifstream file(path_to);

    if (!file) throw std::logic_error("Error: Something with sql file");

    std::stringstream buf;
    buf << file.rdbuf();

    return buf.str();
}

auto database::sql_bool_array(const std::vector<std::optional<std::uint16_t>>
                                  &opt_vec) const -> std::string {

    std::string sql_req = "{";

    for (std::size_t i = 0; i < opt_vec.size(); i++) {

        if (i != 0) sql_req += ',';

        if (opt_vec[i].has_value() && opt_vec[i].value() == 1) sql_req += "t";

        else if (opt_vec[i].has_value() && opt_vec[i].value() == 0)
            sql_req += "f";

        else sql_req += "NULL";
    }

    sql_req += "}";

    return sql_req;
}

auto database::get_id_by_token(const std::string &token) const
    -> std::optional<std::string> {

    try {

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        pqxx::params params{token};

        pqxx::result res = tx.exec(
            "SELECT fk_person_id FROM session WHERE token = $1", params);

        tx.commit();

        auto id_token = res.back()["fk_person_id"].get<std::string>();

        if (!id_token.has_value()) return std::nullopt;

        return id_token;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}

auto database::check_expiration(const std::string &token) const -> bool {

    try {

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        auto t_now = std::chrono::duration_cast<std::chrono::seconds>(
                         (std::chrono::system_clock::now()).time_since_epoch())
                         .count();

        bool res = tx.query_value<bool>(std::format(
            "SELECT * FROM check_token_expiration('{}',{})", token, t_now));

        tx.commit();

        if (!res) return false;

        return true;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return false;
    }
}

auto database::pers_id_with_full_data(const std::string &token) const
    -> std::optional<std::string> {

    auto connection = pool->acquire();

    pqxx::work tx(*connection);

    pqxx::params p1{token};

    auto res = tx.exec(
        "SELECT id, f_name, l_name, phone, citizenship FROM person WHERE id = "
        "(SELECT fk_person_id FROM session WHERE token = $1)",
        p1);

    tx.commit();

    auto fn = res.back()["f_name"].get<std::string>();
    auto ln = res.back()["l_name"].get<std::string>();
    auto p = res.back()["phone"].get<std::string>();
    auto c = res.back()["citizenship"].get<std::string>();

    if (fn == std::nullopt || ln == std::nullopt || p == std::nullopt ||
        c == std::nullopt)

        return std::nullopt;

    return res.back()["id"].get<std::string>();
};

auto database::res_exec(const std::string &p_id, const crow::json::rvalue &json)
    const -> crow::response {

    try {

        std::string h_id = json["hotel_id"].s();
        std::string r_id = json["room_id"].s();

        std::string check_in = json["check_in"].s();
        std::string check_out = json["check_in"].s();


        if (h_id.empty() || r_id.empty() || check_in.empty() ||
            check_out.empty())
            return crow::response(400, "Incorrect data of the order");

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        pqxx::params params{p_id,     h_id,      r_id,
                            check_in, check_out, "inprogress"};

        tx.exec("INSERT INTO orders(fk_person_id, fk_hotel_id, "
                "fk_room_id, check_in, check_out, status) VALUES($1, $2, $3, "
                "$4, $5, $6)",
                params)
            .no_rows();

        tx.commit();

        return crow::response(200);

    } catch (const std::exception &exc) {

        if (std::string(exc.what()) == "cannot find key") {

            std::cout << "Invalid data in the json: " << exc.what()
                      << std::endl;

            return crow::response(400, "Invalid data in the json");
        }

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return crow::response(400, "The connection to the databse failed");
    }
}

auto database::res_log_exec(const std::string &id_pers,
                            const crow::json::rvalue &json) const
    -> crow::response {
    try {

        std::string f_name = json["f_name"].s();
        std::string l_name = json["l_name"].s();
        std::string citizenship = json["citizenship"].s();
        std::string phone = json["phone"].s();
        std::string h_id = json["hotel_id"].s();
        std::string r_id = json["room_id"].s();

        std::string check_in = json["check_in"].s();
        std::string check_out = json["check_in"].s();

        if (f_name.empty() || l_name.empty() || citizenship.empty() ||
            phone.empty() || h_id.empty() || r_id.empty() || check_in.empty() ||
            check_out.empty())
            return crow::response(400, "Incorrect data of the order");

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        pqxx::params params{id_pers,   f_name,      l_name, citizenship,
                            phone,     h_id,        r_id,   check_in,
                            check_out, "inprogress"};

        tx.exec("INSERT INTO orders(fk_person_id,f_name, l_name, citizenship, "
                "phone,fk_hotel_id, fk_room_id, check_in, check_out, status) "
                "VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)",
                params);

        tx.commit();

        return crow::response(200);

    } catch (const std::exception &exc) {

        if (std::string(exc.what()) == "cannot find key") {

            std::cout << "Invalid data in the json: " << exc.what()
                      << std::endl;

            return crow::response(400, "Invalid data in the json");
        }

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return crow::response(400, "The connection to the databse failed");
    }
}

auto database::full_res_exec(const crow::json::rvalue &json) const
    -> std::optional<crow::response> {

    try {

        std::string f_name = json["f_name"].s();
        std::string l_name = json["l_name"].s();
        std::string citizenship = json["citizenship"].s();
        std::string phone = json["phone"].s();
        std::string email = json["email"].s();
        std::string h_id = json["hotel_id"].s();
        std::string r_id = json["room_id"].s();

        std::string check_in = json["check_in"].s();
        std::string check_out = json["check_in"].s();


        if (f_name.empty() || l_name.empty() || citizenship.empty() ||
            phone.empty() || email.empty() || h_id.empty() || r_id.empty() ||
            check_in.empty() || check_out.empty())
            return crow::response(400, "Incorrect data of the order");

        std::stol(check_in);
        std::stol(check_out);

        auto connection = pool->acquire();

        pqxx::work tx(*connection);

        pqxx::params params{f_name,    l_name,      citizenship, phone,
                            email,     h_id,        r_id,        check_in,
                            check_out, "inprogress"};

        tx.exec("INSERT INTO orders(f_name, l_name, citizenship, phone, "
                "email, fk_hotel_id, fk_room_id, check_in, check_out, status) "
                "VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)",
                params);

        tx.commit();

        return crow::response(200);

    } catch (const std::exception &exc) {

        if (std::string(exc.what()) == "cannot find key") {

            std::cout << "Invalid data in the json: " << exc.what()
                      << std::endl;

            return crow::response(400, "Invalid data in the json");
        }

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return crow::response(400, "The connection to the databse failed");
    }
}

auto database::user_reg_exec(const std::string &email,
                             const std::string &pass) const -> crow::response {

    try {

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        std::string salt = crypto::hex(*crypto::rand_bytes(8));

        std::string p_h = crypto::pepper +
                          crypto::hex(crypto::sha256(pass + salt));

        pqxx::params params{email, p_h, salt};

        tx.exec("INSERT INTO person(email, hash_pass, salt) "
                "VALUES($1,$2,$3::text)",
                params);

        tx.commit();

        return crow::response(200);

    } catch (const std::exception &exc) {

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return crow::response(400, "The connection to the databse failed");
    }
}

auto database::user_log_exec(const std::string &email, const std::string &pass)
    const -> std::optional<crow::response> {

    try {

        crow::response r;

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        pqxx::params p1{email};

        pqxx::result res = tx.exec(
            "SELECT salt, hash_pass FROM person WHERE email = $1", p1);

        tx.commit();

        if (res.empty()) return std::nullopt;

        const pqxx::row data = res.back();

        std::string salt = *data["salt"].get<std::string>();
        std::string db_p_h = *data["hash_pass"].get<std::string>();

        std::string p_h = crypto::pepper +
                          crypto::hex(crypto::sha256(pass + salt));

        if (db_p_h == p_h) {

            auto token = crypto::hex(*crypto::rand_bytes(16));

            pqxx::params p2{email};

            pqxx::result r2 = tx.exec(
                "SELECT id FROM person WHERE email = $1::text", p2);

            tx.commit();

            auto id = r2.back()["id"].get<int>();

            const auto t1 =
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

            const auto t2 =
                std::chrono::duration_cast<std::chrono::seconds>(
                    (std::chrono::system_clock::now() + std::chrono::hours(1))
                        .time_since_epoch())
                    .count();

            pqxx::params p3{id, token, t1, t2};

            tx.exec("INSERT INTO session VALUES($1::integer,$2::text, $3, $4)",
                    p3);

            tx.commit();

            r.set_header("Set-Cookie",
                         std::format("sessionid={};HttpOnly; Secure; "
                                     "SameSite=Strict; Path=/; "
                                     "Max-Age=3600",
                                     token));

            r.code = 200;

            return r;
        }

        return std::nullopt;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the databse failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}

auto database::hotels_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        std::string array = sql_bool_array(params.room_params);

        pqxx::params pq_params{
            array,         params.num_for_sort,
            params.person, params.limit,
            params.offset, types_of_orders.find(params.order_by)->second,
            params.order,  params.country,
            params.city};

        pqxx::result res = tx.exec(
            "SELECT * FROM select_hotels_start($1::boolean[] ,$2, $3, $4, "
            "$5, "
            "$6, "
            "$7,$8,$9)",
            pq_params);

        tx.commit();

        const std::size_t num_rows = res.size();

        for (std::size_t i = 0; i < num_rows; i++) {

            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["price"] = nullptr;
            json["user_rating"] = nullptr;
            json["country"] = nullptr;
            json["city"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto price = row["price"].get<double>()) json["price"] = *price;

            if (auto user_rating = row["user_rating"].get<double>())
                json["user_rating"] = *user_rating;

            if (auto country = row["country"].get<std::string>())
                json["country"] = *country;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;


            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    };
}

auto database::hotels_search_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto connection = pool->acquire();

        pqxx::work tx{*connection};

        std::string array = sql_bool_array(params.room_params);

        pqxx::params pq_params{
            array,         params.num_for_sort,
            params.person, params.limit,
            params.offset, types_of_orders.find(params.order_by)->second,
            params.order,  params.country,
            params.city};

        pqxx::result res = tx.exec(
            "SELECT * FROM select_hotels_search($1::boolean[], $2, $3, $4, "
            "$5, "
            "$6, $7, "
            "$8, $9)",
            pq_params);

        tx.commit();

        const std::size_t num_rows = res.size();

        for (std::size_t i = 0; i < num_rows; i++) {

            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["price"] = nullptr;
            json["user_rating"] = nullptr;
            json["stars"] = nullptr;
            json["country"] = nullptr;
            json["city"] = nullptr;
            json["address"] = nullptr;
            json["double_beds"] = nullptr;
            json["single_beds"] = nullptr;
            json["free_cancel"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto price = row["price"].get<double>()) json["price"] = *price;

            if (auto user_rating = row["user_rating"].get<double>())
                json["user_rating"] = *user_rating;

            if (auto stars = row["stars"].get<int>()) json["stars"] = *stars;

            if (auto country = row["country"].get<std::string>())
                json["country"] = *country;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto address = row["hotels_address"].get<std::string>())
                json["address"] = *address;

            if (auto d_beds = row["double_bed"].get<int>())
                json["double_beds"] = *d_beds;

            if (auto s_beds = row["single_bed"].get<int>())
                json["single_beds"] = *s_beds;

            if (auto f_cancel = row["free_cancel"].get<bool>())
                json["free_cancel"] = *f_cancel;

            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;


            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    };
}

auto database::discounted_hotels_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto conn = pool->acquire();

        pqxx::work tx{*conn};

        std::string array = sql_bool_array(params.room_params);

        pqxx::params pq_params{
            array,         params.num_for_sort,
            params.person, params.limit,
            params.offset, types_of_orders.find(params.order_by)->second,
            params.order,  params.country,
            params.city};

        pqxx::result res = tx.exec(
            "SELECT * FROM select_discounted_hotels_start($1, $2, $3, $4, "
            "$5,$6, $7, $8, $9)",
            pq_params);

        tx.commit();

        const std::size_t num_row = res.size();

        for (std::size_t i = 0; i < num_row; i++) {
            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["discount"] = nullptr;
            json["user_rating"] = nullptr;
            json["country"] = nullptr;
            json["city"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto price = row["discount"].get<double>())
                json["discount"] = *price;

            if (auto user_rating = row["user_rating"].get<double>())
                json["user_rating"] = *user_rating;

            if (auto country = row["country"].get<std::string>())
                json["country"] = *country;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;

            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}

auto database::discounted_hotels_search_exec(
    const request_params_t &params) const -> std::optional<crow::json::wvalue> {
    try {
        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto conn = pool->acquire();

        pqxx::work tx{*conn};

        std::string array = sql_bool_array(params.room_params);

        pqxx::params pq_params{
            array,         params.num_for_sort,
            params.person, params.limit,
            params.offset, types_of_orders.find(params.order_by)->second,
            params.order,  params.country,
            params.city};

        pqxx::result res = tx.exec(
            "SELECT * FROM select_discounted_hotels_search($1, $2, $3, $4, "
            "$5,$6, $7, $8, $9)",
            pq_params);

        tx.commit();

        const std::size_t num_row = res.size();

        for (std::size_t i = 0; i < num_row; i++) {

            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["address"] = nullptr;
            json["user_rating"] = nullptr;
            json["stars"] = nullptr;
            json["old_price"] = nullptr;
            json["discount"] = nullptr;
            json["new_price"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto address = row["address"].get<std::string>())
                json["address"] = *address;

            if (auto user_rating = row["user_rating"].get<double>())
                json["user_rating"] = *user_rating;

            if (auto stars = row["stars"].get<int>()) json["stars"] = *stars;

            auto old_price = row["old_price"].get<double>();
            auto discount = row["discount"].get<double>();

            if (old_price != std::nullopt && discount != std::nullopt) {

                json["old_price"] = *old_price;
                json["discount"] = *discount;
                json["new_price"] = *old_price - ((*old_price) * (*discount));
            }
            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;

            list.push_back(json);
        }

        return list;
    }

    catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}


auto database::places_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto conn = pool->acquire();

        pqxx::work tx{*conn};

        pqxx::params pq_params{
            params.num_for_sort, params.limit,
            params.offset,       types_of_orders.find(params.order_by)->second,
            params.order,        params.country,
            params.city};

        pqxx::result res = tx.exec(
            "SELECT * FROM select_places_start($1, $2, $3, $4, $5, $6, $7)",
            pq_params);

        tx.commit();


        const size_t row_num = res.size();

        for (size_t i = 0; i < row_num; i++) {

            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["country"] = nullptr;
            json["city"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto country = row["country"].get<std::string>())
                json["country"] = *country;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;

            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}

auto database::places_search_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        std::optional<std::string> daun;

        auto conn = pool->acquire();

        pqxx::work tx{*conn};

        pqxx::params pq_params{
            params.num_for_sort, params.limit,
            params.offset,       types_of_orders.find(params.order_by)->second,
            params.order,        params.country,
            params.city};

        pqxx::result res = tx.exec("SELECT * FROM select_places_search($1, "
                                   "$2, $3, $4, $5, $6, $7)",
                                   pq_params);

        tx.commit();


        const size_t row_num = res.size();

        for (size_t i = 0; i < row_num; i++) {
            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["country"] = nullptr;
            json["city"] = nullptr;
            json["description"] = nullptr;
            json["user_rating"] = nullptr;
            json["price"] = nullptr;
            json["is_open"] = nullptr;
            json["is_close"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto country = row["country"].get<std::string>())
                json["country"] = *country;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto description = row["description"].get<std::string>())
                json["description"] = *description;

            if (auto user_rating = row["user_rating"].get<double>())
                json["user_rating"] = *user_rating;

            if (auto price = row["price"].get<double>()) json["price"] = *price;

            if (auto is_open = row["is_open"].get<std::string>())
                json["is_open"] = *is_open;

            if (auto is_close = row["is_close"].get<std::string>())
                json["is_close"] = *is_close;

            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;

            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}

auto database::particular_hotel_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto conn = pool->acquire();

        pqxx::work tx{*conn};

        pqxx::result res = tx.exec("SELECT * FROM select_hotel($1)", params.id);

        const size_t row_num = res.size();

        for (size_t i = 0; i < row_num; i++) {
            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["price"] = nullptr;
            json["country"] = nullptr;
            json["city"] = nullptr;
            json["address"] = nullptr;
            json["stars"] = nullptr;
            json["description_of_the_hotel"] = nullptr;
            json["climate_control"] = nullptr;
            json["high_speed_wifi"] = nullptr;
            json["p_bathroom"] = nullptr;
            json["tv_add_opt"] = nullptr;
            json["mini_bar"] = nullptr;
            json["in_room_safe"] = nullptr;
            json["work_features"] = nullptr;
            json["room_service"] = nullptr;
            json["room_cleaning"] = nullptr;
            json["transport_service"] = nullptr;
            json["luggage_storage"] = nullptr;
            json["full_time_supp"] = nullptr;
            json["fitness_center"] = nullptr;
            json["in_room_workout_equip"] = nullptr;
            json["spa"] = nullptr;
            json["swimming_pool_cov"] = nullptr;
            json["swimming_pool_unc"] = nullptr;
            json["business_center"] = nullptr;
            json["meeting_rooms"] = nullptr;
            json["breakfast_opt"] = nullptr;
            json["in_room_dining"] = nullptr;
            json["double_bed"] = nullptr;
            json["single_bed"] = nullptr;
            json["child_service"] = nullptr;
            json["price_child_service"] = nullptr;
            json["pet_service"] = nullptr;
            json["price_pet_service"] = nullptr;
            json["free_cancel"] = nullptr;
            json["group_of"] = nullptr;
            json["description_of_type"] = nullptr;
            json["photo_path"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto price = row["price"].get<double>()) json["price"] = *price;

            if (auto country = row["country"].get<std::string>())
                json["country"] = *country;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto address = row["address"].get<std::string>())
                json["address"] = *address;

            if (auto stars = row["stars"].get<int>()) json["stars"] = *stars;

            if (auto description_of_the_hotel =
                    row["description_of_the_hotel"].get<std::string>())
                json["description_of_the_hotel"] = *description_of_the_hotel;

            if (auto climate_control = row["climate_control"].get<bool>())
                json["climate_control"] = *climate_control;

            if (auto high_speed_wifi = row["high_speed_wifi"].get<bool>())
                json["high_speed_wifi"] = *high_speed_wifi;

            if (auto p_bathroom = row["p_bathroom"].get<bool>())
                json["p_bathroom"] = *p_bathroom;

            if (auto tv_add_opt = row["tv_add_opt"].get<bool>())
                json["tv_add_opt"] = *tv_add_opt;

            if (auto mini_bar = row["mini_bar"].get<bool>())
                json["mini_bar"] = *mini_bar;

            if (auto in_room_safe = row["in_room_safe"].get<bool>())
                json["in_room_safe"] = *in_room_safe;

            if (auto work_features = row["work_features"].get<bool>())
                json["work_features"] = *work_features;

            if (auto room_service = row["room_service"].get<bool>())
                json["room_service"] = *room_service;

            if (auto room_cleaning = row["room_cleaning"].get<bool>())
                json["room_cleaning"] = *room_cleaning;

            if (auto transport_service = row["transport_service"].get<bool>())
                json["transport_service"] = *transport_service;

            if (auto luggage_storage = row["luggage_storage"].get<bool>())
                json["luggage_storage"] = *luggage_storage;

            if (auto full_time_supp = row["full_time_supp"].get<bool>())
                json["full_time_supp"] = *full_time_supp;

            if (auto fitness_center = row["fitness_center"].get<bool>())
                json["fitness_center"] = *fitness_center;

            if (auto in_room_workout_equip =
                    row["in_room_workout_equip"].get<bool>())
                json["in_room_workout_equip"] = *in_room_workout_equip;

            if (auto spa = row["spa"].get<bool>()) json["spa"] = *spa;

            if (auto swimming_pool_cov = row["swimming_pool_cov"].get<bool>())
                json["swimming_pool_cov"] = *swimming_pool_cov;

            if (auto swimming_pool_unc = row["swimming_pool_unc"].get<bool>())
                json["swimming_pool_unc"] = *swimming_pool_unc;

            if (auto business_center = row["business_center"].get<bool>())
                json["business_center"] = *business_center;

            if (auto meeting_rooms = row["meeting_rooms"].get<bool>())
                json["meeting_rooms"] = *meeting_rooms;

            if (auto breakfast_opt = row["breakfast_opt"].get<bool>())
                json["breakfast_opt"] = *breakfast_opt;

            if (auto in_room_dining = row["in_room_dining"].get<bool>())
                json["in_room_dining"] = *in_room_dining;

            if (auto double_bed = row["double_bed"].get<int>())
                json["double_bed"] = *double_bed;

            if (auto single_bed = row["single_bed"].get<int>())
                json["single_bed"] = *single_bed;

            if (auto child_service = row["child_service"].get<bool>())
                json["child_service"] = *child_service;

            if (auto price_child_service =
                    row["price_child_service"].get<double>())
                json["price_child_service"] = *price_child_service;

            if (auto pet_service = row["pet_service"].get<bool>())
                json["pet_service"] = *pet_service;

            if (auto price_pet_service = row["price_pet_service"].get<double>())
                json["price_pet_service"] = *price_pet_service;

            if (auto free_cancel = row["free_cancel"].get<bool>())
                json["free_cancel"] = *free_cancel;

            if (auto group_of = row["group_of"].get<std::uint32_t>())
                json["group_of"] = *group_of;

            if (auto description_of_type =
                    row["description_of_type"].get<std::string>())
                json["description_of_type"] = *description_of_type;

            if (auto photo_path = row["photos_path"].get<std::string>())
                json["photo_path"] = *photo_path;

            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}

auto database::particular_place_exec(const request_params_t &params) const
    -> std::optional<crow::json::wvalue> {

    try {

        crow::json::wvalue json;
        crow::json::wvalue::list list;

        auto conn = pool->acquire();

        pqxx::work tx{*conn};

        pqxx::result res = tx.exec("SELECT * FROM select_place($1)", params.id);

        const size_t row_num = res.size();

        for (std::size_t i = 0; i < row_num; i++) {

            const pqxx::row row = res[i];

            json["name"] = nullptr;
            json["city"] = nullptr;
            json["address"] = nullptr;
            json["user_rating"] = nullptr;
            json["price"] = nullptr;
            json["description"] = nullptr;
            json["photos_path"] = nullptr;
            json["hotels"] = nullptr;

            if (auto name = row["name"].get<std::string>())
                json["name"] = *name;

            if (auto city = row["city"].get<std::string>())
                json["city"] = *city;

            if (auto address = row["address"].get<std::string>())
                json["address"] = *address;

            if (auto user_rating = row["user_rating"].get<double>())
                json["user_rating"] = *user_rating;

            if (auto price = row["price"].get<double>()) json["price"] = *price;

            if (auto description = row["description"].get<std::string>())
                json["description"] = *description;

            if (auto photos_path = row["photos_path"].get<std::string>())
                json["photos_path"] = *photos_path;

            auto hotel = row["hotel"].as_sql_array<std::string, 1>();

            std::vector<std::string> vec;

            for (std::size_t i = 0; i < hotel.size(); i++) {

                vec.push_back(hotel[i]);
            }

            json["hotels"] = vec;

            list.push_back(json);
        }

        return list;

    } catch (const std::exception &exc) {

        std::cout << "The connection to the database failed: " << exc.what()
                  << std::endl;

        return std::nullopt;
    }
}