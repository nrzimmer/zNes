cmake_minimum_required(VERSION 3.30)
project(znes C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -pedantic -O0)

find_package(raylib REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(znes src/main.c
        src/bus.c
        src/bus.h
        src/cpu.c
        src/cpu.h
        src/ppu.c
        src/ppu.h
        src/cartridge.c
        src/cartridge.h
        src/mappers/mapper.c
        src/mappers/mapper.h
        src/mappers/mapper_000.c
        src/mappers/mapper_000.h
        src/forward.h
)

target_link_libraries(znes PRIVATE raylib)