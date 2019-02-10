cmake_minimum_required(VERSION 3.0)

project(stonejs2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(APID_USE_LIBUV ON)

add_subdirectory(APID EXCLUDE_FROM_ALL)
add_subdirectory(StoneAPI EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME} SHARED src/main.cpp)
target_compile_definitions(${PROJECT_NAME} PUBLIC APID_USE_LIBUV=1)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_link_libraries(${PROJECT_NAME} stone-api ${CMAKE_JS_LIB})
