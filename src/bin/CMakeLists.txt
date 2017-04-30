PROJECT(OutLander)
cmake_minimum_required(VERSION 2.6)

ADD_EXECUTABLE(olc olc.cxx)
TARGET_LINK_LIBRARIES(olc ol boost_program_options)
INSTALL(TARGETS olc DESTINATION bin)

# ADD_EXECUTABLE(ols ols.cxx)
# TARGET_LINK_LIBRARIES(ols ol boost_program_options)
# INSTALL(TARGETS ols DESTINATION bin)
