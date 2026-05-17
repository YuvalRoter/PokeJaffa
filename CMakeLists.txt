cmake_minimum_required(VERSION 3.31)
project(PokeJaffa)

set(CMAKE_CXX_STANDARD 20)

add_executable(PokeJaffa
        bagel.h
        main.cpp
        PokemonModel.h
        PokemonModel.cpp)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
add_subdirectory(lib/SDL)
target_link_libraries(PokeJaffa PUBLIC SDL3-static)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/SDL_image)
target_link_libraries(PokeJaffa PUBLIC SDL3_image-static)

if(MINGW)
# Forces the console subsystem to the absolute end of the compiler command
target_link_options(PokeJaffa PRIVATE "-Wl,--subsystem,console")
endif()