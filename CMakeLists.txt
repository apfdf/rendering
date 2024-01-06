cmake_minimum_required(VERSION 3.5.0)
project(rendering VERSION 0.1.0)

add_executable(rendering src/main.cpp)

target_link_libraries(rendering glfw GLEW GL)