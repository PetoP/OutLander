PROJECT(OutLander)
cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE(OTB)
IF(OTB_FOUND)
INCLUDE(${OTB_USE_FILE})
ELSE(OTB_FOUND)
MESSAGE(FATAL_ERROR
"Cannot build OTB project without OTB.
 Please set OTB_DIR.")
ENDIF(OTB_FOUND)
# SET(CMAKE_CXX_FLAGS "-std=c++14")

ADD_LIBRARY(OutLanderLibrary oll.cpp)
TARGET_LINK_LIBRARIES(OutLanderLibrary ${OTB_LIBRARIES})
TARGET_LINK_LIBRARIES(OutLanderLibrary boost_system)
TARGET_LINK_LIBRARIES(OutLanderLibrary boost_filesystem)
ADD_EXECUTABLE(OutLander ol.cpp)
TARGET_LINK_LIBRARIES(OutLander OutLanderLibrary)
# TARGET_LINK_LIBRARIES(OutLander svm)
