cmake_minimum_required(VERSION 3.21)

project(GameEngine VERSION 1.0.0 LANGUAGES CXX)
if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This distribution targets Windows x64.")
endif()

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
"Modules/Editor.cpp"
"Modules/Editor.h"
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
find_package(imguizmo CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(assimp REQUIRED)
find_package(pugixml CONFIG REQUIRED)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCE})

add_executable(GameEngine ${ENGINE_SOURCE})
target_compile_features(GameEngine PRIVATE cxx_std_17)

target_link_libraries(GameEngine PRIVATE SDL3::SDL3)
target_link_libraries(GameEngine PRIVATE glad::glad)
target_link_libraries(GameEngine PRIVATE glm::glm)
target_link_libraries(GameEngine PRIVATE imgui::imgui imguizmo::imguizmo)
target_link_libraries(GameEngine PRIVATE DevIL::IL)
target_link_libraries(GameEngine PRIVATE DevIL::ILU)
target_link_libraries(GameEngine PRIVATE DevIL::ILUT)
target_link_libraries(GameEngine PRIVATE assimp::assimp)
target_link_libraries(GameEngine PRIVATE pugixml::shared pugixml::pugixml)
target_link_libraries(GameEngine PRIVATE psapi)


# A distributable contains runtime files only; build logs and tools stay in build/.
install(TARGETS GameEngine
    RUNTIME_DEPENDENCIES
        DIRECTORIES "$<TARGET_FILE_DIR:GameEngine>"
        PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
        POST_EXCLUDE_REGEXES ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\].*"
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .)
install(DIRECTORY Assets DESTINATION .)
install(FILES config.xml DESTINATION .)

# Ship the release Visual C++ runtime locally: no installer or admin access required.
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
set(CMAKE_INSTALL_DEBUG_LIBRARIES OFF)
include(InstallRequiredSystemLibraries)
if(MSVC AND NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    message(FATAL_ERROR "The redistributable Visual C++ runtime was not found; packaging would be incomplete.")
endif()

# Keep the original third-party license notices alongside the distributed binaries.
file(GLOB dependency_notices "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/*/copyright")
foreach(notice IN LISTS dependency_notices)
    get_filename_component(package_directory "${notice}" DIRECTORY)
    get_filename_component(package_name "${package_directory}" NAME)
    install(FILES "${notice}" DESTINATION "Licenses/${package_name}")
endforeach()

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME GameEngine)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "GameEngine-${PROJECT_VERSION}-Windows-x64")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")
include(CPack)
