cmake_minimum_required(VERSION 3.5)

project(byte-stuffing-tryouts LANGUAGES C)

add_executable(byte-stuffing-tryouts
    ${CMAKE_SOURCE_DIR}/main.c
    ${CMAKE_SOURCE_DIR}/cobs/cobs.c
    ${CMAKE_SOURCE_DIR}/cobs/cobsr.c
    ${CMAKE_SOURCE_DIR}/tcobs/tcobs.c
    ${CMAKE_SOURCE_DIR}/rle/rle.c
)

target_include_directories(byte-stuffing-tryouts PRIVATE
    ${CMAKE_SOURCE_DIR}/cobs
    ${CMAKE_SOURCE_DIR}/tcobs
    ${CMAKE_SOURCE_DIR}/rle
)
