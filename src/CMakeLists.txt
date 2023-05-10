find_package(SEAL 4.0 REQUIRED)

add_library(sealpir pir.hpp pir.cpp pir_client.hpp pir_client.cpp pir_server.hpp
  pir_server.cpp)
target_link_libraries(sealpir SEAL::seal_shared)

add_library(adpir server.cpp user.hpp user.cpp ad.hpp ad.cpp)
target_link_libraries(adpir sealpir)

add_executable(main main.cpp)
target_link_libraries(main adpir)

add_executable(client_base client_base.cpp)
add_executable(server_base server_base.cpp)
