PROJECT(OutLander)
cmake_minimum_required(VERSION 2.6)

ADD_LIBRARY(ol SHARED oll.cxx)
TARGET_LINK_LIBRARIES(ol ${OTB_LIBRARIES} ${ITK_LIBRARIES} gdal boost_system boost_filesystem)
INSTALL(FILES oll.hxx DESTINATION include)
INSTALL(TARGETS ol DESTINATION lib)

ADD_EXECUTABLE(outlander ol.cxx)
TARGET_LINK_LIBRARIES(outlander ol)
install(TARGETS outlander DESTINATION bin)
