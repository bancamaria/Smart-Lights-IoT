cmake_minimum_required(VERSION 3.16)
project(Smart_Lights_IoT_Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)
include(FetchContent)

FetchContent_Declare(json
        GIT_REPOSITORY  https://github.com/nlohmann/json.git
        GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(pistache
        GIT_REPOSITORY https://github.com/pistacheio/pistache)
FetchContent_GetProperties(pistache)
if(NOT pistache_POPULATED)
    FetchContent_Populate(pistache)
    add_subdirectory(${pistache_SOURCE_DIR} ${pistache_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()



#find_package(Pistache 0.0.2 REQUIRED)
add_executable(Smart_Lights_IoT_Project main.cpp SmartLightController.cpp SmartLightController.h SmartLamp.cpp SmartLamp.h)
target_link_libraries(Smart_Lights_IoT_Project pistache_shared)

target_link_libraries( Smart_Lights_IoT_Project nlohmann_json::nlohmann_json)