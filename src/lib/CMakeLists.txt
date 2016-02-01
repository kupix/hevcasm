cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../../xbyak")

if(MSVC)
#We statically link to reduce dependancies
foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
    if(${flag_var} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MDd")
endforeach(flag_var)
endif(MSVC)

add_library (libhevcasm diff.cpp hadamard.cpp hevcasm.cpp hevcasm_test.cpp pred_inter.cpp pred_intra.cpp quantize.cpp residual_decode.cpp sad.cpp ssd.cpp)
target_link_libraries(libhevcasm)
target_include_directories (libhevcasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(GCC_COMPILE_FLAGS "-fno-operator-names -std=c++0x -O3")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
