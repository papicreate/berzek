cmake_minimum_required(VERSION 3.16)
project(berzek-web LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_executable(berzek src/main.cpp)

# Put built web outputs under build/web
set_target_properties(berzek PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web"
  OUTPUT_NAME "index"
)

# Emscripten-specific flags
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  message(STATUS "Configuring target for Emscripten")
  target_compile_options(berzek PRIVATE -std=c++17)
  target_link_options(berzek PRIVATE
    "-s" "WASM=1"
    "-s" "ALLOW_MEMORY_GROWTH=1"
    "-s" "USE_WEBGL2=1"
    "-s" "ASSERTIONS=1"
    "-O2"
    "-s" "EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
  )
endif()

# Find SFML (must be compiled for Emscripten)
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

target_include_directories(berzek PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(berzek PRIVATE sfml-graphics sfml-window sfml-system)
