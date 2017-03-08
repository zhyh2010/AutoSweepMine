cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0012 NEW)
PROJECT(AutoSweepMine)
FIND_PACKAGE(OpenCV 3.0.0 REQUIRED)
MESSAGE("OpenCV version : ${OpenCV_VERSION}")
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})

SET(SRC main.cpp MineGame.cpp MineGame.h unitTest/unitTest.h unitTest/unitTest.cpp)
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenCV_LIBS} UnitTest)

