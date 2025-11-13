cmake_minimum_required(VERSION 3.14)
project(GameboyEmu C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# Optional flag to control SDL2 fetching
option(USE_FETCHCONTENT "Automatically fetch SDL2 if not found" ON)

if(USE_FETCHCONTENT)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.26.6 # change to desired version
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL_LIBS SDL2::SDL2 SDL2::SDL2main)
    include_directories(${SDL2_SOURCE_DIR}/include)
else()
    # Try to find installed SDL2
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    set(SDL_LIBS ${SDL2_LIBRARIES})
endif()

# Source files
file(GLOB SRC
    main.c
    memory/*.c
    cpu/*.c
    io/*.c
    core/*.c
    debug/*.c
)

# Include directories (for headers)
include_directories(
    cpu
    io
    core
    memory
    debug
)

# Build executable
add_executable(gbemu ${SRC})

# Link SDL2 if available
target_link_libraries(gbemu ${SDL_LIBS})
