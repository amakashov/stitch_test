cmake_minimum_required(VERSION 3.5.0)
project(stitch_test VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} )
message("OpenCV include dir ${OpenCV_INCLUDE_DIRS}")

find_package(GDAL CONFIG REQUIRED)

file(GLOB src *.cpp *.hpp)
add_executable(stitch_test ${src})
target_include_directories(stitch_test PRIVATE ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(stitch_test PUBLIC ${OpenCV_LIBS} )
target_link_libraries(stitch_test PRIVATE GDAL::GDAL)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
