cmake_minimum_required(VERSION 3.15)

project(UnitTest)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g2 -ggdb")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -g2 -ggdb")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ./bin)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(UnitTest main.cpp)

target_include_directories(UnitTest PRIVATE ${SRC_DIR}/include/openssl)

target_link_directories(UnitTest PRIVATE ${SRC_DIR}/lib/openssl)

find_library(SSL NAMES libssl.a)
find_library(CRYPTO NAMES libcrypto.a)

target_link_libraries(UnitTest PRIVATE ${SSL})
target_link_libraries(UnitTest PRIVATE ${CRYPTO})
target_link_libraries(UnitTest PRIVATE dl)
target_link_libraries(UnitTest PRIVATE gtest)
target_link_libraries(UnitTest PRIVATE pthread)
target_link_libraries(UnitTest PRIVATE mysqlclient)
target_link_libraries(UnitTest PRIVATE z)

file(COPY ${SRC_DIR}/cert.key DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${SRC_DIR}/cert.pem DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${SRC_DIR}/wwwroot DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${SRC_DIR}/view DESTINATION ${EXECUTABLE_OUTPUT_PATH})
