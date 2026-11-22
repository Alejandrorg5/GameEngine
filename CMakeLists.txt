cmake_minimum_required(VERSION 3.21)

option(BUILD_EDITOR "Build the E1 editor" ON)
if(BUILD_EDITOR)
    list(APPEND VCPKG_MANIFEST_FEATURES editor)
endif()

project(GameEngine VERSION 1.0.0 LANGUAGES CXX)
if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This distribution targets Windows x64.")
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(assimp REQUIRED)
find_package(pugixml CONFIG REQUIRED)

add_library(EngineCore STATIC
    Core/Camera.cpp
    Core/Grid.cpp
    Core/Mesh.cpp
    Core/Components/TextureComponent.cpp
    Core/GameObject.cpp
    Core/ModelLoader.cpp
    Core/TextureLoader.cpp
    Core/Components/MeshComponent.cpp
    Modules/Engine.cpp
    Modules/Input.cpp
    Modules/Log.cpp
    Modules/PerfTimer.cpp
    Modules/Render.cpp
    Modules/RunEngine.cpp
    Modules/Scene.cpp
    Modules/Timer.cpp
    Modules/Window.cpp)
target_compile_features(EngineCore PUBLIC cxx_std_17)
target_link_libraries(EngineCore PUBLIC SDL3::SDL3 glad::glad glm::glm pugixml::pugixml assimp::assimp
    PRIVATE DevIL::IL)

# Launching either executable directly from the build directory uses the same E1 assets.
configure_file(config.xml config.xml COPYONLY)
file(GLOB_RECURSE runtime_assets CONFIGURE_DEPENDS LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Assets/*")
foreach(asset IN LISTS runtime_assets)
    configure_file("${asset}" "${asset}" COPYONLY)
endforeach()

add_executable(GameRuntime Core/GameMain.cpp)
target_link_libraries(GameRuntime PRIVATE EngineCore)
set(distribution_target GameRuntime)
if(BUILD_EDITOR)
    find_package(imgui CONFIG REQUIRED)
    find_package(imguizmo CONFIG REQUIRED)
    add_executable(GameEngine Core/main.cpp Modules/Editor.cpp)
    target_link_libraries(GameEngine PRIVATE EngineCore imgui::imgui imguizmo::imguizmo DevIL::IL psapi)
    set(distribution_target GameEngine)
endif()

# Check the configuration of the binaries, even if CPack was invoked with -C Release.
if(CMAKE_CONFIGURATION_TYPES)
    install(CODE [[
        if(NOT CMAKE_INSTALL_CONFIG_NAME STREQUAL "Release")
            message(FATAL_ERROR "The E1 package requires a Release build.")
        endif()
    ]])
elseif(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    install(CODE [[message(FATAL_ERROR "The E1 package requires a Release build.")]])
endif()
install(TARGETS ${distribution_target}
    RUNTIME_DEPENDENCIES
        DIRECTORIES "$<TARGET_FILE_DIR:${distribution_target}>"
        PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
        POST_EXCLUDE_REGEXES ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\].*"
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .)
install(DIRECTORY Assets DESTINATION .)
install(FILES config.xml LICENCE.md DESTINATION .)
if(BUILD_EDITOR)
    install(FILES README.md DESTINATION .)
    set(E1_TASK_LIST "" CACHE FILEPATH "E1 task list to include in the submission")
    if(E1_TASK_LIST)
        install(FILES "${E1_TASK_LIST}" DESTINATION .)
    endif()
endif()

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
    if(package_name MATCHES "^vcpkg-")
        continue()
    endif()
    install(FILES "${notice}" DESTINATION "Licenses/${package_name}")
endforeach()

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME "${distribution_target}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${distribution_target}-${PROJECT_VERSION}-Windows-x64")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckPackage.cmake")
include(CPack)
