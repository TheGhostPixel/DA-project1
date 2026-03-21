cmake_minimum_required(VERSION 4.1)
project(DA_Project_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(DA_Project_1 main.cpp
        parser.cpp
        parser.h
        DataModels.h
)
