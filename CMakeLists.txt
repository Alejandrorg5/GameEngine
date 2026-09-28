cmake_minimum_required(VERSION 3.10)

project(EngineProject)

set(ENGINE_SOURCE 
"Core/main.cpp"
"Modules/Render.cpp"
"Modules/Module.h")

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(DevIL REQUIRED)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCE})

add_executable(GameEngine ${ENGINE_SOURCE})

target_link_libraries(GameEngine PRIVATE SDL3::SDL3)
target_link_libraries(GameEngine PRIVATE glad::glad)
target_link_libraries(GameEngine PRIVATE glm::glm)
target_link_libraries(GameEngine PRIVATE DevIL::IL)
target_link_libraries(GameEngine PRIVATE DevIL::ILU)
target_link_libraries(GameEngine PRIVATE DevIL::ILUT)
	