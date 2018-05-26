CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

PROJECT(cfdata)
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

ADD_SUBDIRECTORY(third-party/json)
ADD_SUBDIRECTORY(third-party/curlpp)

ADD_EXECUTABLE(cfdata main.cpp entities.hpp timer.hpp queue.hpp)
TARGET_LINK_LIBRARIES(cfdata curlpp)

TARGET_INCLUDE_DIRECTORIES(cfdata PRIVATE third-party/curlpp/include)
TARGET_INCLUDE_DIRECTORIES(cfdata PRIVATE third-party/json/single_include)
