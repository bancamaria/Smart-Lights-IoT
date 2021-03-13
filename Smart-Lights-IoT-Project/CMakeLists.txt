cmake_minimum_required(VERSION 3.16)
project(Smart_Lights_IoT_Project)

set(CMAKE_CXX_STANDARD 14)
find_package(Pistache 0.0.2 REQUIRED)

add_executable(Smart_Lights_IoT_Project main.cpp)
target_link_libraries(Smart_Lights_IoT_Project pistache_shared)