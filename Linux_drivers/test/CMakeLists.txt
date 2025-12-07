cmake_minimum_required(VERSION 3.10)
project(isp_pipeline)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
add_executable(isp_pipeline
    main.cpp
)

target_include_directories(isp_pipeline PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(isp_pipeline ${OpenCV_LIBS})
