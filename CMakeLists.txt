cmake_minimum_required(VERSION 3.10)
project(SuperMario VERSION 0.99 LANGUAGES C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

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
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			)
ExternalProject_Get_Property(SFML INSTALL_DIR)
set(SFML_LIB ${INSTALL_DIR})

ExternalProject_Add(TinyXml2 
		#GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
		SOURCE_DIR  ${CMAKE_SOURCE_DIR}/ext/tinyxml2
    	CMAKE_ARGS 
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>)
ExternalProject_Get_Property(TinyXml2 INSTALL_DIR)
set(TINY_XML_LIB ${INSTALL_DIR})

# ADD SOURCE FILES AND HEADERS OF EXECUTABLE
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${SFML_LIB}/include ${TINY_XML_LIB}/include)

set(SOURCE
	${CMAKE_SOURCE_DIR}/src/Main.cpp
	${CMAKE_SOURCE_DIR}/src/Blocks.cpp
	${CMAKE_SOURCE_DIR}/src/GameEngine.cpp
	${CMAKE_SOURCE_DIR}/src/Geometry.cpp
	${CMAKE_SOURCE_DIR}/src/Items.cpp
	${CMAKE_SOURCE_DIR}/src/Mario.cpp
	${CMAKE_SOURCE_DIR}/src/Enemies.cpp
	${CMAKE_SOURCE_DIR}/src/Pickups.cpp
	${CMAKE_SOURCE_DIR}/src/SuperMarioGame.cpp
)

set(HEADERS
	${CMAKE_SOURCE_DIR}/include/Blocks.h
	${CMAKE_SOURCE_DIR}/include/GameEngine.h
	${CMAKE_SOURCE_DIR}/include/Geometry.h
	${CMAKE_SOURCE_DIR}/include/Items.h
	${CMAKE_SOURCE_DIR}/include/Mario.h
	${CMAKE_SOURCE_DIR}/include/Enemies.h
	${CMAKE_SOURCE_DIR}/include/Pickups.h
	${CMAKE_SOURCE_DIR}/include/SuperMarioGame.h
)
 
# LINK EXTERNAL LIBRARIES TO EXECUTABLE
LINK_DIRECTORIES(${SFML_LIB}/lib ${TINY_XML_LIB}/lib)

# ADD EXECUTABLE
add_executable(SuperMario ${SOURCE} ${HEADERS})
if (MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SuperMario)
	set_target_properties( SuperMario PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Build")
	set_target_properties( SuperMario PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Build")
endif(MSVC)

# ADD DEPENDISIES FOR EXECUTABLE
add_dependencies(SuperMario SFML TinyXml2)

TARGET_LINK_LIBRARIES(SuperMario 
					optimized sfml-system		debug sfml-system-d 
					optimized sfml-window		debug sfml-window-d 
					optimized sfml-graphics		debug sfml-graphics-d 
					optimized sfml-audio		debug sfml-audio-d 
					optimized tinyxml2			debug tinyxml2d)                

# POST BUILD SCRIPTS
set(PRE_LIB_DIR "lib")
if (WIN32)
	set(POST_LIB_DIR "bin")
endif()

ADD_CUSTOM_COMMAND(TARGET SuperMario POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${SFML_LIB}/${POST_LIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TINY_XML_LIB}/${POST_LIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/res" "${CMAKE_CURRENT_BINARY_DIR}/res")

