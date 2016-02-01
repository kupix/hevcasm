cmake_minimum_required(VERSION 2.8)
add_executable (hevcasm main.cpp)
target_link_libraries (hevcasm LINK_PUBLIC libhevcasm)
