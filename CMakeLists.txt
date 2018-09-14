cmake_minimum_required(VERSION 3.0.0)
project(ADXL355_Sample VERSION 0.0.0)

include(CTest)
enable_testing()

option(build_sample "Build sample executables" ON)

add_library(ADXL355_I2C ADXL355.cpp ADXL355.h)
set_target_properties(ADXL355_I2C PROPERTIES OUTPUT_NAME ADXL355_I2C)

if (build_sample)
	add_executable(ADXL355_Sample main.cpp ADXL355.h)
	set_target_properties(ADXL355_Sample PROPERTIES OUTPUT_NAME ADXL355_Sample)
	target_link_libraries(ADXL355_Sample ADXL355_I2C wiringPi)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
