cmake_minimum_required(VERSION 3.26)
project(adpir)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SEAL 4.1.1 REQUIRED)

add_library(sealpir pir.hpp pir.cpp pir_client.hpp pir_client.cpp pir_server.hpp
  pir_server.cpp)
target_link_libraries(sealpir SEAL::seal_shared)

add_library(adpir server.hpp server.cpp user.hpp user.cpp ad.hpp ad.cpp)
target_link_libraries(adpir sealpir)

add_executable(main main.cpp)
target_link_libraries(main adpir)
