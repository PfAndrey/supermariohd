include(SFML3)
include(TinyXML2)

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Collisions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TimerManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Property.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StateMachine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Vector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Format.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Collisions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TimerManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GameObject.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Property.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Rect.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StateMachine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Vector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Format.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/RTIIX.hpp
)

add_library(game-framework STATIC
    ${SOURCE}
    ${HEADERS}
)

target_include_directories(game-framework
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/
)

target_link_libraries(game-framework
    SFML3::all
    TinyXML2::TinyXML2
)
