cmake_minimum_required(VERSION 3.5.0)
project(portals VERSION 0.1.0)

add_executable(portals src/main.cpp)

target_link_libraries(portals glfw GLEW GL)