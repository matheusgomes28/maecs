find_package(unordered_dense REQUIRED)
find_package(Microsoft.GSL REQUIRED)

add_library(maecs)
target_sources(maecs
  PUBLIC
    FILE_SET CXX_MODULES FILES
      maecs.cpp
)

target_link_libraries(maecs
 PRIVATE
  Microsoft.GSL::GSL
  maecs::compile_time
  unordered_dense::unordered_dense)

# if(BUILD_PROFILER)
#   message(STATUS "Building profiler")
#   target_link_libraries(emulator PRIVATE profiler)
#   target_compile_definitions(emulator PRIVATE BUILD_PROFILER=)
# else()
#   message(STATUS "Profiler not build")
# endif()

add_library(maecs::maecs ALIAS maecs)
