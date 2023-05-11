set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.81.0 COMPONENTS log REQUIRED)
find_package(SEAL 4.1.1 REQUIRED)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})

add_library(sealpir pir.hpp pir.cpp pir_client.hpp pir_client.cpp pir_server.hpp
  pir_server.cpp)
target_link_libraries(sealpir SEAL::seal_shared)

add_library(adpir user.hpp user.cpp ad.hpp ad.cpp 
  custom_serialization.cpp custom_serialization.hpp)
target_link_libraries(adpir sealpir)

add_executable(main_user main_user.cpp)
target_link_libraries(main_user adpir)
target_link_libraries(main_user ${Boost_LIBRARIES})

add_executable(main_server main_server.cpp)
target_link_libraries(main_server adpir)
target_link_libraries(main_server ${Boost_LIBRARIES})

add_executable(client_base client_base.cpp)
add_executable(server_base server_base.cpp)
