find_package(SEAL 4.1.1 REQUIRED)

add_library(sealpir pir.hpp pir.cpp pir_client.hpp pir_client.cpp pir_server.hpp
  pir_server.cpp)
target_link_libraries(sealpir SEAL::seal_shared)

add_library(adpir server.hpp server.cpp user.hpp user.cpp ad.hpp ad.cpp)
target_link_libraries(adpir sealpir)

add_executable(main main.cpp)
target_link_libraries(main adpir)

add_executable(server server.cpp)
target_link_libraries(server adpir)
