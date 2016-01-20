cmake_minimum_required(VERSION 2.8)
add_library (libhevcasm diff.c hadamard.c hevcasm.c hevcasm_test.c pred_inter.c pred_intra.c quantize.c residual_decode.c sad.c ssd.c)
target_include_directories (libhevcasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
