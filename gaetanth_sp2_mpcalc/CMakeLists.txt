cmake_minimum_required(VERSION 3.15)

project(mpcalc VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(mpcalc 
    Semestralka_2/main.cpp
    Semestralka_2/MpInt.h
    Semestralka_2/MpTerm.h)

target_include_directories(mpcalc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Semestralka_2)