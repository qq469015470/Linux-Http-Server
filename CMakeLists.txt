cmake_minimum_required(VERSION 3.18.1)

project(HttpServer)

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ./bin)

add_executable(HttpServer src/main.cpp)

target_include_directories(HttpServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include/openssl)

target_link_directories(HttpServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/openssl)

find_library(SSL NAMES libssl.a)
find_library(CRYPTO NAMES libcrypto.a)

target_link_libraries(HttpServer PRIVATE ${SSL})
target_link_libraries(HttpServer PRIVATE ${CRYPTO})
target_link_libraries(HttpServer PRIVATE dl)
target_link_libraries(HttpServer PRIVATE pthread)
target_link_libraries(HttpServer PRIVATE mysqlclient)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cert.key DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cert.pem DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/wwwroot DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/view DESTINATION ${EXECUTABLE_OUTPUT_PATH})
