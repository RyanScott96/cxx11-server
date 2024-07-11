cmake_minimum_required(VERSION 3.5)

project("httcpp")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPPILE_COMMANDS ON)

add_executable(main src/main.cpp src/threadpool.cpp src/http.cpp src/mime.cpp)
target_include_directories(main PRIVATE include/)
