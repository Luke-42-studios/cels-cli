# cels-cli -- Interactive TUI toolkit for CELS
# Built with CELS itself using cels-ncurses backend
cmake_minimum_required(VERSION 3.21)

add_executable(cels-cli
    src/main.c
    src/app.c
    src/scaffold.c
)

set_target_properties(cels-cli PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

target_include_directories(cels-cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cels-cli PRIVATE
    cels
    cels-widgets
    cels-ncurses
    flecs::flecs_static
)
