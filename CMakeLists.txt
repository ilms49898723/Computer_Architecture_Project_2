cmake_minimum_required(VERSION 3.5)

project(Computer_Architecture_Project_2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Os")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -Wall -Wextra")

set(SOURCE_FILES
        InstDataBin.cpp
        InstDataBin.h
        InstDataStr.cpp
        InstDataStr.h
        InstDecoder.cpp
        InstDecoder.h
        InstErrorDetector.cpp
        InstErrorDetector.h
        InstImageReader.cpp
        InstImageReader.h
        InstLookUp.cpp
        InstLookUp.h
        InstMemory.cpp
        InstMemory.h
        InstPipelineData.cpp
        InstPipelineData.h
        InstSimulator.cpp
        InstSimulator.h
        InstType.h
        InstUtility.cpp
        InstUtility.h
        main.cpp)

add_executable(pipeline ${SOURCE_FILES})
