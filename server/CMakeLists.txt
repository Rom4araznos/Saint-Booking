add_definitions(-DCROW_ENABLE_SSL)

add_executable(server_bk src/httpserver.cpp src/routes.cpp src/database.cpp src/database_pool.cpp src/core.cpp src/crypto.cpp src/main.cpp)

target_compile_definitions(server_bk PRIVATE CROW_ENABLE_SSL)

find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libpqxx REQUIRED)

add_custom_command(
  TARGET server_bk
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/server/bin_sql/functions"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/server/sql/functions"
          "${CMAKE_BINARY_DIR}/server/bin_sql/functions"
  COMMENT "Copying SQL folder to server build directory"
)

target_include_directories(server_bk PRIVATE include ${PostgreSQL_INCLUDE_DIRS} ${LIBPQXX_INCLUDE_DIRS})

target_link_libraries(server_bk PRIVATE Crow::Crow PostgreSQL::PostgreSQL ${LIBPQXX_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
