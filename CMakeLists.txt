cmake_minimum_required(VERSION 3.5)

project(HttpServer)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g2 -ggdb")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -g2 -ggdb")
endif()

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
target_link_libraries(HttpServer PRIVATE dw)
target_link_libraries(HttpServer PRIVATE pthread)
target_link_libraries(HttpServer PRIVATE mysqlclient)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cert.key DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/cert.pem DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/wwwroot DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/view DESTINATION ${EXECUTABLE_OUTPUT_PATH})
