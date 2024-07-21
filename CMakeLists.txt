cmake_minimum_required(VERSION 3.28)
project(Maecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_CXX_VISIBILITY_PRESET hidden)
# set(CMAKE_VISIBILITY_INLINES_HIDDEN On)

option(BUILD_PROFILER "Build profiling for the instruction handlers")

add_subdirectory(compile_time)
add_subdirectory(ecs_app)
add_subdirectory(maecs)

if (BUILD_PROFILER)
    add_subdirectory(profiler)
endif()

enable_testing()
add_subdirectory(tests)
