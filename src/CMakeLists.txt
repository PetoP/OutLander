PROJECT(OutLander)
cmake_minimum_required(VERSION 2.6)

# ol library
ADD_LIBRARY(ol SHARED olcl.cxx trainingSitesContainer.cxx trainingSite.cxx classStatistics.cxx olsl.cxx)
TARGET_LINK_LIBRARIES(ol ${OTB_LIBRARIES} ${ITK_LIBRARIES} gdal boost_system boost_filesystem)
INSTALL(FILES olcl.hxx DESTINATION include)
INSTALL(TARGETS ol DESTINATION lib)

# ol classifier
ADD_EXECUTABLE(olc  olc.cxx)
TARGET_LINK_LIBRARIES(olc ol)
TARGET_LINK_LIBRARIES(olc boost_program_options)
install(TARGETS olc DESTINATION bin)

# ol statist
ADD_EXECUTABLE(ols ols.cxx)
TARGET_LINK_LIBRARIES(ols ol)
TARGET_LINK_LIBRARIES(ols boost_program_options)
install(TARGETS ols DESTINATION bin)
