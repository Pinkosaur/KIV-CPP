cmake_minimum_required(VERSION 3.10)

project(drawing VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(/wd4244 /wd4101)

add_executable(drawing
    Semestralka_1/main.cpp
    Semestralka_1/FileWriter.h
    Semestralka_1/Instruction.h
    Semestralka_1/Translate.h
    Semestralka_1/Rotate.h
    Semestralka_1/Scale.h
    Semestralka_1/Line.h
    Semestralka_1/Rectangle.h
    Semestralka_1/Circle.h
)

target_include_directories(drawing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Semestralka_1)
