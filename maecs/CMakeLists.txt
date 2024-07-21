find_package(unordered_dense REQUIRED)
find_package(Microsoft.GSL REQUIRED)

add_library(maecs)
target_sources(maecs
 PUBLIC
  maecs.cpp
)

target_include_directories(maecs PUBLIC include)

target_link_libraries(maecs
 PUBLIC
  Microsoft.GSL::GSL
  maecs::compile_time
  unordered_dense::unordered_dense
)

add_library(maecs::maecs ALIAS maecs)
