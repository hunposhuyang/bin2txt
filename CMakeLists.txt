cmake_minimum_required(VERSION 3.10.0)
project(bin2txt VERSION 0.1.0 LANGUAGES C CXX)

add_executable(bin2txt main.cpp)
#设置可执行文件名称和路径，路径为项目根目录
set_target_properties(bin2txt PROPERTIES
        OUTPUT_NAME bin2txt
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

#设置c++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
