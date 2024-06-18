
add_library(compile_time)
target_sources(compile_time
  PUBLIC
    FILE_SET CXX_MODULES FILES
      compile_time.cpp
)

add_library(maecs::compile_time ALIAS compile_time)
