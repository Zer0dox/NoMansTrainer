cmake_minimum_required(VERSION 3.26)
project(NoMansTrainer)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(NoMansTrainer
        memproc.cpp
        memproc.h
        nms.cpp
        nms.h)
