# Simple FetchContent for tinyxml2
include(FetchContent)

set(TINYXML2_TAG "10.0.0" CACHE STRING "tinyxml2 tag to fetch")

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  tinyxml2
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
  GIT_TAG        ${TINYXML2_TAG}
)

# Download and build
FetchContent_MakeAvailable(tinyxml2)

add_library(TinyXML2::TinyXML2 ALIAS tinyxml2)
