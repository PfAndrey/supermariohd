cmake_minimum_required(VERSION 3.8)

project(SuperMario VERSION 0.99 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# BUILD EXTERNAL LIBRARIES
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
ExternalProject_Add(SFML
        #GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.5.1
        SOURCE_DIR  ${CMAKE_SOURCE_DIR}/ext/SFML
            CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
        ExternalProject_Get_Property(SFML INSTALL_DIR)
set(SFML_LIB ${INSTALL_DIR})

ExternalProject_Add(TinyXml2
        #GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        SOURCE_DIR  ${CMAKE_SOURCE_DIR}/ext/tinyxml2
            CMAKE_ARGS
            -DBUILD_TESTS=OFF
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>)
ExternalProject_Get_Property(TinyXml2 INSTALL_DIR)
set(TINY_XML_LIB ${INSTALL_DIR})

link_directories(${SFML_LIB}/lib ${TINY_XML_LIB}/lib)

set(MARIO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)

set(SOURCE
    ${MARIO_SOURCE_DIR}/game_framework/Collisions.cpp
    ${MARIO_SOURCE_DIR}/game_framework/InputManager.cpp
    ${MARIO_SOURCE_DIR}/game_framework/TimerManager.cpp
    ${MARIO_SOURCE_DIR}/game_framework/GameObject.cpp
    ${MARIO_SOURCE_DIR}/game_framework/Property.cpp
    ${MARIO_SOURCE_DIR}/game_framework/StateMachine.cpp
    ${MARIO_SOURCE_DIR}/game_framework/Logger.cpp
    ${MARIO_SOURCE_DIR}/game_framework/Rect.cpp
    ${MARIO_SOURCE_DIR}/game_framework/Vector.cpp
    ${MARIO_SOURCE_DIR}/utils/Format.cpp
    ${MARIO_SOURCE_DIR}/Blocks.cpp
    ${MARIO_SOURCE_DIR}/GameEngine.cpp
    ${MARIO_SOURCE_DIR}/Items.cpp
    ${MARIO_SOURCE_DIR}/Mario.cpp
    ${MARIO_SOURCE_DIR}/enemies/Blooper.cpp
    ${MARIO_SOURCE_DIR}/enemies/Bowser.cpp
    ${MARIO_SOURCE_DIR}/enemies/BulletBill.cpp
    ${MARIO_SOURCE_DIR}/enemies/BuzzyBeetle.cpp
    ${MARIO_SOURCE_DIR}/enemies/CheepCheep.cpp
    ${MARIO_SOURCE_DIR}/enemies/Enemy.cpp
    ${MARIO_SOURCE_DIR}/enemies/Fireball.cpp
    ${MARIO_SOURCE_DIR}/enemies/Goomba.cpp
    ${MARIO_SOURCE_DIR}/enemies/HammerBro.cpp
    ${MARIO_SOURCE_DIR}/enemies/Koopa.cpp
    ${MARIO_SOURCE_DIR}/enemies/Lakity.cpp
    ${MARIO_SOURCE_DIR}/enemies/PiranhaPlant.cpp
    ${MARIO_SOURCE_DIR}/enemies/Podoboo.cpp
    ${MARIO_SOURCE_DIR}/pickups/Coin.cpp
    ${MARIO_SOURCE_DIR}/pickups/Fireflower.cpp
    ${MARIO_SOURCE_DIR}/pickups/Mushroom.cpp
    ${MARIO_SOURCE_DIR}/pickups/Star.cpp
    ${MARIO_SOURCE_DIR}/SuperMarioGame.cpp
    ${MARIO_SOURCE_DIR}/main.cpp
)

set(HEADERS
    ${MARIO_SOURCE_DIR}/game_framework/Collisions.hpp
    ${MARIO_SOURCE_DIR}/game_framework/InputManager.hpp
    ${MARIO_SOURCE_DIR}/game_framework/TimerManager.hpp
    ${MARIO_SOURCE_DIR}/game_framework/GameObject.hpp
    ${MARIO_SOURCE_DIR}/game_framework/Logger.hpp
    ${MARIO_SOURCE_DIR}/game_framework/Property.hpp
    ${MARIO_SOURCE_DIR}/game_framework/Rect.hpp
    ${MARIO_SOURCE_DIR}/game_framework/StateMachine.hpp
    ${MARIO_SOURCE_DIR}/game_framework/ResourceManager.hpp
    ${MARIO_SOURCE_DIR}/game_framework/Vector.hpp
    ${MARIO_SOURCE_DIR}/utils/Format.hpp
    ${MARIO_SOURCE_DIR}/utils/RTIIX.hpp
    ${MARIO_SOURCE_DIR}/Character.hpp
    ${MARIO_SOURCE_DIR}/TileMap.hpp
    ${MARIO_SOURCE_DIR}/Blocks.hpp
    ${MARIO_SOURCE_DIR}/GameEngine.hpp
    ${MARIO_SOURCE_DIR}/Items.hpp
    ${MARIO_SOURCE_DIR}/Mario.hpp
    ${MARIO_SOURCE_DIR}/enemies/Blooper.hpp
    ${MARIO_SOURCE_DIR}/enemies/Bowser.hpp
    ${MARIO_SOURCE_DIR}/enemies/BulletBill.hpp
    ${MARIO_SOURCE_DIR}/enemies/BuzzyBeetle.hpp
    ${MARIO_SOURCE_DIR}/enemies/CheepCheep.hpp
    ${MARIO_SOURCE_DIR}/enemies/Enemy.hpp
    ${MARIO_SOURCE_DIR}/enemies/Enemies.hpp
    ${MARIO_SOURCE_DIR}/enemies/Fireball.hpp
    ${MARIO_SOURCE_DIR}/enemies/Goomba.hpp
    ${MARIO_SOURCE_DIR}/enemies/HammerBro.hpp
    ${MARIO_SOURCE_DIR}/enemies/Koopa.hpp
    ${MARIO_SOURCE_DIR}/enemies/Lakity.hpp
    ${MARIO_SOURCE_DIR}/enemies/PiranhaPlant.hpp
    ${MARIO_SOURCE_DIR}/enemies/Podoboo.hpp
    ${MARIO_SOURCE_DIR}/pickups/Pickups.hpp
    ${MARIO_SOURCE_DIR}/pickups/Coin.hpp
    ${MARIO_SOURCE_DIR}/pickups/Fireflower.hpp
    ${MARIO_SOURCE_DIR}/pickups/Mushroom.hpp
    ${MARIO_SOURCE_DIR}/pickups/Star.hpp
    ${MARIO_SOURCE_DIR}/SuperMarioGame.hpp
)

# Target
add_executable(SuperMario ${SOURCE} ${HEADERS})

# Include directories
target_include_directories(SuperMario
    PRIVATE
    ${MARIO_SOURCE_DIR}/
    ${MARIO_SOURCE_DIR}/enemies
    ${MARIO_SOURCE_DIR}/game_framework
    ${MARIO_SOURCE_DIR}/pickups
    ${MARIO_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/include
    ${SFML_LIB}/include
    ${TINY_XML_LIB}/include
)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SuperMario)
    set_target_properties( SuperMario PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Build")
    set_target_properties( SuperMario PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Build")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

    file(GLOB_RECURSE GAME_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/source/*.c"
    "${CMAKE_SOURCE_DIR}/source/*.hpp"
    "${CMAKE_SOURCE_DIR}/source/*.h"
)

source_group(TREE "${CMAKE_SOURCE_DIR}/source" FILES ${GAME_FILES})


endif(MSVC)

# ADD DEPENDISIES FOR EXECUTABLE
add_dependencies(SuperMario SFML TinyXml2)

#find_library(MYLIB1 NAMES mylibrary1 PATHS /path/to/libraries REQUIRED)

target_link_libraries(SuperMario
    optimized sfml-system       debug sfml-system-d
    optimized sfml-window       debug sfml-window-d
    optimized sfml-graphics     debug sfml-graphics-d
    optimized sfml-audio        debug sfml-audio-d
    optimized tinyxml2          debug tinyxml2d
)

if (UNIX AND NOT APPLE)
    target_link_libraries(SuperMario  stdc++ m pthread)
endif()

# POST BUILD SCRIPTS
set(POST_LIB_DIR "lib")
if (WIN32)
    set(POST_LIB_DIR "bin")
endif()

ADD_CUSTOM_COMMAND(TARGET SuperMario POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SFML_LIB}/${POST_LIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${TINY_XML_LIB}/${POST_LIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/res" "${CMAKE_CURRENT_BINARY_DIR}/res")

