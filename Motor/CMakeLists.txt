cmake_minimum_required(VERSION 3.10)

project(EngineProject)

set(src "src/GameEngine.cpp")

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(DevIL REQUIRED)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${src})

add_executable(GameEngine ${src})

target_link_libraries(GameEngine PRIVATE SDL3::SDL3)
target_link_libraries(GameEngine PRIVATE glad::glad)
target_link_libraries(GameEngine PRIVATE glm::glm)
target_link_libraries(GameEngine PRIVATE DevIL::IL)
target_link_libraries(GameEngine PRIVATE DevIL::ILU)
target_link_libraries(GameEngine PRIVATE DevIL::ILUT)
