cmake_minimum_required(VERSION 4.0)

project(text-adventure)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE
    src/main.cpp
)
