PROJECT(OutLander)
cmake_minimum_required(VERSION 2.6)

# ADD_LIBRARY(ol SHARED classStatistics.cxx olcl.cxx olsl.cxx trainingSite.cxx)
ADD_LIBRARY(ol SHARED olcl.cxx)
TARGET_LINK_LIBRARIES(ol ${OTB_LIBRARIES} ${ITK_LIBRARIES} gdal boost_system boost_filesystem)

INSTALL(TARGETS ol DESTINATION lib)
INSTALL(FILES olcl.hxx DESTINATION include)
# INSTALL(FILES olsl.hxx DESTINATION include)
