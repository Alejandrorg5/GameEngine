cmake_minimum_required(VERSION 3.10)

project(EngineProject)

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(GameEngine GameEngine.cpp)

target_link_libraries(GameEngine PRIVATE SDL3::SDL3)
target_link_libraries(GameEngine PRIVATE glad::glad)
