cmake_minimum_required(VERSION 3.20)
project(CodotakuSDL VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/std:c++latest /permissive-)
endif()

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
set(GLMBUILD_TESTS OFF CACHE INTERNAL "")
set(GLMBUILD_INSTALL OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glm)

if(NOT TARGET SDL3::SDL3)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.8
        GIT_SHALLOW TRUE
    )
    set(SDL_STATIC ON CACHE INTERNAL "")
    set(SDL_SHARED OFF CACHE INTERNAL "")
    set(SDL_TEST_LIBRARY OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(SDL3)
endif()

add_library(codotaku
    src/runtime.cpp
)

target_include_directories(codotaku
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(codotaku
    PUBLIC
        glm::glm
        SDL3::SDL3
)



add_executable(triangle
    examples/triangle.cpp
)

target_link_libraries(triangle PRIVATE codotaku)
