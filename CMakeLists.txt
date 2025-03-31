cmake_minimum_required(VERSION 3.10.0)
project(bin2txt VERSION 0.1.0 LANGUAGES C CXX)

add_executable(bin2txt main.cpp)

#设置c++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
