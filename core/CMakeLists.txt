cmake_minimum_required(VERSION 3.24)

include(FetchContent)
include(ExternalProject)

option(EMSCRIPTEN "Build to empscripten")


set(EXTRA_CXX_FLAGS "-D__EMSCRIPTEN_STANDALONE_WASM__=1")
# set(EXTRA_CXX_FLAGS "-D__EMSCRIPTEN_STANDALONE_WASM__=1 -pthread")

list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=install")
list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS -DCMAKE_CXX_FLAGS=${EXTRA_CXX_FLAGS})
list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS -DCMAKE_C_FLAGS=${EXTRA_CXX_FLAGS})
if (EMSCRIPTEN)
  list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=$ENV{EMSCRIPTEN_ROOT}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
  list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_CROSSCOMPILING_EMULATOR=$ENV{EMSCRIPTEN_ROOT}/node/16.20.0_64bit/bin/node")
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(SED_INLINE '')
else()
  set(SED_INLINE)
endif()

ExternalProject_Add(kvazaar 
  GIT_REPOSITORY https://github.com/ultravideo/kvazaar.git
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/kvazaar-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/kvazaar-build
  TEST_COMMAND      ""
  PATCH_COMMAND     sed -i ${SED_INLINE} "/-muse-unaligned-vector-move/d" ${CMAKE_BINARY_DIR}/kvazaar-src/CMakeLists.txt
  CMAKE_ARGS        -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
)

ExternalProject_Add(libde265
  GIT_REPOSITORY https://github.com/strukturag/libde265.git
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/libde265-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/libde265-build
  TEST_COMMAND      ""
  CMAKE_ARGS        -DENABLE_SDL=OFF -DENABLE_DECODER=OFF -DBUILD_SHARED_LIBS=OFF ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
)

set(LIBDE265_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libde265-build/install/include)
set(LIBDE265_LIBRARY_DIR ${CMAKE_BINARY_DIR}/libde265-build/libde265)
set(LIBDE265_LIBRARY ${LIBDE265_LIBRARY_DIR}/libde265.a)
set(KVAZAAR_INCLUDE_DIR ${CMAKE_BINARY_DIR}/kvazaar-build/install/include)
set(KVAZAAR_LIBRARY_DIR ${CMAKE_BINARY_DIR}/kvazaar-build/install/lib)
set(KVAZAAR_LIBRARY ${KVAZAAR_LIBRARY_DIR}/libkvazaar.a)
set(LIBHEIF_LIBRARY_DIR ${CMAKE_BINARY_DIR}/libheif-build/install/lib)

ExternalProject_Add(
  libheif
  GIT_REPOSITORY    https://github.com/strukturag/libheif.git
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/libheif-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/libheif-build
  TEST_COMMAND      ""
  CMAKE_ARGS        -DWITH_KVAZAAR=ON -DWITH_GDK_PIXBUF=OFF -DWITH_EXAMPLES=OFF -DENABLE_MULTITHREADING_SUPPORT=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF -DLIBDE265_INCLUDE_DIR=${LIBDE265_INCLUDE_DIR} -DLIBDE265_LIBRARY=${LIBDE265_LIBRARY} -DKVAZAAR_INCLUDE_DIR=${KVAZAAR_INCLUDE_DIR} -DKVAZAAR_LIBRARY=${KVAZAAR_LIBRARY} ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
)

set(CMAKE_CXX_STANDARD 17)

add_library(elheif STATIC src/impl.cpp)
add_dependencies(elheif kvazaar libde265 libheif)
add_dependencies(libheif kvazaar libde265)
target_include_directories(elheif PUBLIC include)
target_include_directories(elheif PRIVATE ${CMAKE_BINARY_DIR}/libheif-build/install/include)
target_link_libraries(elheif PUBLIC
  # -pthread
  -lheif
  -lde265
  -lkvazaar
  -L${KVAZAAR_LIBRARY_DIR}
  -L${LIBDE265_LIBRARY_DIR}
  -L${LIBHEIF_LIBRARY_DIR}
)

