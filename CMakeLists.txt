cmake_minimum_required(VERSION 3.5)

project(TcobsBench LANGUAGES C)

add_executable(TcobsBench
    ${CMAKE_SOURCE_DIR}/main.c
    ${CMAKE_SOURCE_DIR}/cobs/cobsr.c
    ${CMAKE_SOURCE_DIR}/tcobs/tcobs.c
)

target_include_directories(TcobsBench PRIVATE
    ${CMAKE_SOURCE_DIR}/cobs
    ${CMAKE_SOURCE_DIR}/tcobs
)
