cmake_minimum_required(VERSION 3.2)

project(
    StrashAPIClient
    VERSION 0.0
    LANGUAGES CXX
)

include(FetchContent)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu++0x -D__STDC_INT64__")


find_package(OpenSSL REQUIRED)

find_package(Boost 1.81 REQUIRED)

find_package(nlohmann_json REQUIRED)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(SOURCES
    src/ConfigReader.cpp
    src/ApiClient.cpp
    src/Query.cpp
    src/ThreadPool.cpp
)
set(HEADERS
    includes/ConfigReader.h
    includes/ApiClient.h
    includes/Query.h
    includes/QueryFactory.h
    includes/utils.h
    includes/ThreadPool.h
)

add_library(
    ${PROJECT_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE includes)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json Boost::boost OpenSSL::SSL OpenSSL::Crypto)


SET(STRASHAPICLIENT_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
add_subdirectory(tests)
