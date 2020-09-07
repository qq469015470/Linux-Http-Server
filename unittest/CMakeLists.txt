cmake_minimum_required(VERSION 3.18.1)

project(UnitTest)

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ./bin)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(UnitTest main.cpp JsonObjTest.cpp)

target_include_directories(UnitTest PRIVATE ${SRC_DIR}/include/openssl)

target_link_directories(UnitTest PRIVATE ${SRC_DIR}/lib/openssl)

find_library(SSL NAMES libssl.a)
find_library(CRYPTO NAMES libcrypto.a)

target_link_libraries(UnitTest PRIVATE ${SSL})
target_link_libraries(UnitTest PRIVATE ${CRYPTO})
target_link_libraries(UnitTest PRIVATE dl)
target_link_libraries(UnitTest PRIVATE gtest)
target_link_libraries(UnitTest PRIVATE pthread)

file(COPY ${SRC_DIR}/cert.key DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${SRC_DIR}/cert.pem DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${SRC_DIR}/wwwroot DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${SRC_DIR}/view DESTINATION ${EXECUTABLE_OUTPUT_PATH})
