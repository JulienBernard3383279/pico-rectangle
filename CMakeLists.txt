cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(rectangle)

pico_sdk_init()

include_directories(include)

add_executable(rectangle
    src/main.c
    src/comms.c
    src/logic.c
)

pico_add_extra_outputs(rectangle)

target_link_libraries(rectangle pico_stdlib)
