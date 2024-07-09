
find_package(benchmark REQUIRED)

add_executable(performance_bench main.cpp)

target_link_libraries(performance_bench
 PRIVATE
  maecs
  benchmark::benchmark)

add_subdirectory(registry_tests)