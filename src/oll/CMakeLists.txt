PROJECT(OutLander)
cmake_minimum_required(VERSION 2.6)

ADD_LIBRARY(ol SHARED classStatistics.cxx olcl.cxx olcl.hxx olsl.cxx olsl.hxx trainingSite.cxx trainingSitesContainer.cxx)
TARGET_LINK_LIBRARIES(ol ${OTB_LIBRARIES} ${ITK_LIBRARIES} gdal boost_system boost_filesystem)
