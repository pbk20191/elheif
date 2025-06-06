cmake_minimum_required(VERSION 3.14)
include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(elheif-wasm src/impl.cpp)
target_compile_definitions(elheif-wasm PUBLIC)
add_dependencies(elheif-wasm elheif)
target_link_libraries(elheif-wasm PRIVATE elheif
    # -pthread
    -lembind
    # -oelheif.mjs
    -sINITIAL_HEAP=4MB
    -sEXPORT_NAME=MainModuleFactory
    # -sSINGLE_FILE=1
    -sENVIRONMENT=web,worker
    -sEXPORT_ES6=1
    -sALLOW_MEMORY_GROWTH=1
    # -sPTHREAD_POOL_SIZE=1
    # -sPTHREAD_POOL_SIZE_STRICT=2
    -sMODULARIZE
    -sSTACK_SIZE=1MB
    --profiling
    # -sMINIMAL_RUNTIME=2
    # -sSTANDALONE_WASM=1
    -flto
    -sMALLOC=emmalloc
    -sFILESYSTEM=0
)
target_link_libraries(elheif-wasm PRIVATE "--post-js=${CMAKE_CURRENT_SOURCE_DIR}/src/post.js")
target_link_libraries(elheif-wasm PRIVATE "--pre-js=${CMAKE_CURRENT_SOURCE_DIR}/src/pre.js")
target_link_libraries(elheif-wasm PRIVATE "--emit-tsd elheif-wasm.d.ts")

# https://github.com/clangd/clangd/issues/1621
# make clangd work with emscripten
execute_process(COMMAND em++ --cflags OUTPUT_VARIABLE EM_CFLAGS)
message("${EM_CFLAGS}")
set_target_properties(elheif-wasm PROPERTIES COMPILE_FLAGS "${EM_CFLAGS} ")
