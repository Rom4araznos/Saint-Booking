cmake_minimum_required(VERSION 3.24)
project("Saint-Booking")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)    

include(cmake/CPM.cmake)

CPMAddPackage(
NAME crow
GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
VERSION 1.2.0
)

CPMAddPackage(
    NAME libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.10.0
)


find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(server/)
add_subdirectory(client/)