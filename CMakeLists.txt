# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
    cels-clay
    cels-ncurses
    flecs::flecs_static
)
