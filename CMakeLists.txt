cmake_minimum_required(VERSION 3.10)

project(EngineProject)

set(ENGINE_SOURCE
"Core/main.cpp"
"Core/Mesh.h"
"Core/Grid.h"
"Core/ModelLoader.h"
"Core/TextureLoader.h"
"Core/GameObject.cpp"
"Core/GameObject.h"
"Core/Camera.h"
"Core/Components/Component.h"
"Core/Components/TransformComponent.h"
"Core/Components/TextureComponent.h"
"Core/Components/MeshComponent.cpp"
"Core/Components/MeshComponent.h"
"Modules/Engine.cpp"
"Modules/Engine.h"
"Modules/Input.cpp"
"Modules/Input.h"
"Modules/Log.cpp"
"Modules/Log.h"
"Modules/Module.h"
"Modules/PerfTimer.cpp"
"Modules/PerfTimer.h"
"Modules/Render.cpp"
"Modules/Render.h"
"Modules/Scene.cpp"
"Modules/Scene.h"
"Modules/Timer.cpp"
"Modules/Timer.h"
"Modules/Window.cpp"
"Modules/Window.h"
)

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(assimp REQUIRED)
find_package(pugixml CONFIG REQUIRED)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCE})

add_executable(GameEngine ${ENGINE_SOURCE})

target_link_libraries(GameEngine PRIVATE SDL3::SDL3)
target_link_libraries(GameEngine PRIVATE glad::glad)
target_link_libraries(GameEngine PRIVATE glm::glm)
target_link_libraries(GameEngine PRIVATE imgui::imgui)
target_link_libraries(GameEngine PRIVATE DevIL::IL)
target_link_libraries(GameEngine PRIVATE DevIL::ILU)
target_link_libraries(GameEngine PRIVATE DevIL::ILUT)
target_link_libraries(GameEngine PRIVATE assimp::assimp)
target_link_libraries(GameEngine PRIVATE pugixml::shared pugixml::pugixml)