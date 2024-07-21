find_package(raylib REQUIRED)

add_executable(ecs_app main.cpp)
target_link_libraries(ecs_app
 PRIVATE
  maecs::maecs
  raylib)
