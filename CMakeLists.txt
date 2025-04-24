cmake_minimum_required(VERSION 3.10)
project(final_project VERSION 0.1.0 LANGUAGES C)

# Set static linking flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# Ensure static libraries are used
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")

set(SOURCES
    main.c
    bbb_dht_read.c
    bbb_mmio.c
    common_dht_read.c
)

set(HEADERS
    bbb_dht_read.h
    bbb_mmio.h
    common_dht_read.h
)

add_executable(final_project ${SOURCES} ${HEADERS})