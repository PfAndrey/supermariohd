cmake_minimum_required(VERSION 3.21)

project(SuperMario VERSION 0.99 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


set(MARIO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/game_framework)

set(SOURCE
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
add_executable(SuperMario)

target_sources(SuperMario
PRIVATE
    ${SOURCE}
    ${HEADERS}
)

# Include directories
target_include_directories(SuperMario
    PRIVATE
    ${MARIO_SOURCE_DIR}/
    ${MARIO_SOURCE_DIR}/enemies
    ${MARIO_SOURCE_DIR}/game_framework
    ${MARIO_SOURCE_DIR}/pickups
)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SuperMario)
    set_target_properties( SuperMario PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Build")
    set_target_properties( SuperMario PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Build")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

    source_group(TREE "${MARIO_SOURCE_DIR}" FILES ${SOURCE} ${HEADERS})

endif(MSVC)

target_link_libraries(SuperMario
    PRIVATE
    game-framework
)

if (UNIX AND NOT APPLE)
    target_link_libraries(SuperMario
    PRIVATE
    m
    pthread)
endif()

add_custom_command(TARGET SuperMario POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
          "${CMAKE_SOURCE_DIR}/res"
          "$<TARGET_FILE_DIR:SuperMario>/res"
)
