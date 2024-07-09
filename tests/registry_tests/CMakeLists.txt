find_package(GTest REQUIRED)

add_executable(registry_tests)
target_sources(registry_tests
 PRIVATE
  registry_tests.cpp)

target_link_libraries(registry_tests
 PRIVATE
  maecs::maecs
  GTest::gtest_main
)
