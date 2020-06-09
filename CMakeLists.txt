cmake_minimum_required(VERSION 3.14)
project(cpp_ftp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)

include_directories(/usr/local/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(cpp_ftp globals.cpp utils.cpp ftptransfer.cpp argparse.hpp ftp.cpp main.cpp)

add_executable(test_ftp globals.cpp utils.cpp ftptransfer.cpp ftp.cpp testftp.cpp )

add_subdirectory(filesystem)
add_subdirectory(sockpp)
# if sockpp is installed, then uncomment the following line
# and comment out the previous line (add_subdirectory(sockpp))
# find_library(SOCKPP sockpp)

target_link_libraries(cpp_ftp ghc_filesystem)
target_link_libraries(test_ftp ghc_filesystem)
target_link_libraries(cpp_ftp sockpp)
target_link_libraries(test_ftp sockpp)
# if sockpp is installed, then uncomment the following line
# and comment out the previous two lines (target_link_libraries(cpp_ftp sockpp))
# target_link_libraries(cpp_ftp "${SOCKPP}")
# target_link_libraries(test_ftp "${SOCKPP}")
