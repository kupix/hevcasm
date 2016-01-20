cmake_minimum_required(VERSION 3.0)
add_library (libhevcasm diff.c hadamard.c hevcasm.c hevcasm_test.c pred_inter.c pred_intra.c quantize.c residual_decode.c sad.c ssd.c)
target_include_directories (libhevcasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET libhevcasm PROPERTY C_STANDARD 99)