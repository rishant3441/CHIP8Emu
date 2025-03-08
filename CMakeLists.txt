cmake_minimum_required(VERSION 3.25)
project(CHIP8Emu)
set(SDL2_DIR "D:\\Dev\\SDL2\\SDL2-2.0.22")
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 20)

add_executable(CHIP8Emu main.cpp Chip8Emu.cpp Chip8Emu.h)

target_link_libraries(CHIP8Emu PRIVATE ${SDL2_LIBRARY})