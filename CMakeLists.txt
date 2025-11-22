cmake_minimum_required(VERSION 3.16)
project(berzek LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(berzek berzek.cpp)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
if (SFML_FOUND)
    target_link_libraries(berzek PRIVATE sfml-graphics sfml-window sfml-system)
endif()

# If a font.ttf exists in the source dir, copy it next to the executable after build
add_custom_command(TARGET berzek POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/font.ttf
        $<TARGET_FILE_DIR:berzek>/font.ttf
    COMMENT "Copying font.ttf to executable directory (if present)")
