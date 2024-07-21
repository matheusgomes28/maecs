
add_library(compile_time INTERFACE)
target_sources(compile_time
 PUBLIC
  include/compile_time/compile_time.hpp
)

target_include_directories(compile_time INTERFACE include)

add_library(maecs::compile_time ALIAS compile_time)
