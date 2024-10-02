cmake_minimum_required(VERSION 3.22)
project(Cxxurwa_Calculator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include_directories(src)

set(SRC
   src/main.cpp 
)

add_compile_options(-DMYTEST)

add_executable(execute ${SRC})
