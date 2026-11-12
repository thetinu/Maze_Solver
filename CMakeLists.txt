cmake_minimum_required(VERSION 3.16)
project(MazeSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where SFML is installed (adjust if you extracted SFML elsewhere)
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "C:/SFML")
endif()

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

add_executable(MazeSolver
    src/main.cpp
    src/Maze.cpp
    src/Algorithms.cpp
    src/Visualizer.cpp
)

target_include_directories(MazeSolver PRIVATE include)
target_link_libraries(MazeSolver PRIVATE sfml-graphics sfml-window sfml-system)

# Copy SFML DLLs next to the executable so it runs without PATH tweaks
add_custom_command(TARGET MazeSolver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_PREFIX_PATH}/bin"
        "$<TARGET_FILE_DIR:MazeSolver>"
)