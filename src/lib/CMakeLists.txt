cmake_minimum_required(VERSION 3.0)

Set(ASMJIT_EMBED TRUE)
Add_Definitions(-DASMJIT_STATIC)

Set(ASMJIT_DIR "${CMAKE_SOURCE_DIR}/asmjit")
Include("${ASMJIT_DIR}/CMakeLists.txt")

include_directories("${ASMJIT_DIR}/src")

add_library (libhevcasm diff.cpp hadamard.cpp hevcasm.cpp hevcasm_test.cpp pred_inter.cpp pred_intra.cpp quantize.cpp residual_decode.cpp sad.cpp ssd.cpp ${ASMJIT_SRC})
Target_Link_Libraries(libhevcasm ${ASMJIT_DEPS})
target_include_directories (libhevcasm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
