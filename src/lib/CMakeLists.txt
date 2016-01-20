cmake_minimum_required(VERSION 3.0)
add_library (libhevcasm diff.cpp hadamard.cpp hevcasm.cpp hevcasm_test.cpp pred_inter.cpp pred_intra.cpp quantize.cpp residual_decode.cpp sad.cpp ssd.cpp)
target_include_directories (libhevcasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
