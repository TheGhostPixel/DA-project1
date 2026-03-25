cmake_minimum_required(VERSION 3.16)
project(DA_Project_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(DA_Project_1.2
        main.cpp
        parser.cpp
        parser.h
        DataModels.h
        Graph.h
        MaxFlow.cpp
        MaxFlow.h
        AssignmentSolver.cpp
        AssignmentSolver.h
        RiskAnalysis.cpp
        RiskAnalysis.h

)