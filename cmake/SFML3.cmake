# Simple FetchContent for SFML 3.x
include(FetchContent)

set(SFML2_TAG "3.0.1" CACHE STRING "SFML 2.x tag to fetch")

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC      OFF CACHE BOOL "" FORCE)
# set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  sfml3
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG        ${SFML2_TAG}
)

# Download and build
FetchContent_MakeAvailable(sfml3)

add_library(SFML3::all INTERFACE IMPORTED)
target_link_libraries(SFML3::all INTERFACE
  sfml-graphics
  sfml-audio
  # sfml-network
)

