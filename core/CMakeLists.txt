cmake_minimum_required(VERSION 3.24)

include(FetchContent)
include(ExternalProject)

option(EMSCRIPTEN "Build to empscripten")
option(EMS_EXTRA "Add Extra support" OFF)

if(MSVC)
  # Specify the runtime library for MSVC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")  # Static runtime (use /MD for dynamic)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
endif()

set(EXTRA_CXX_FLAGS "-D__EMSCRIPTEN_STANDALONE_WASM__=1")
# set(EXTRA_CXX_FLAGS "-D__EMSCRIPTEN_STANDALONE_WASM__=1 -pthread")

list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=install")
list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS -DCMAKE_CXX_FLAGS=${EXTRA_CXX_FLAGS})
list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS -DCMAKE_C_FLAGS=${EXTRA_CXX_FLAGS})
if (EMSCRIPTEN)
  list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
  list(APPEND EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS "-DCMAKE_CROSSCOMPILING_EMULATOR=$ENV{EMSDK_NODE}")
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

ExternalProject_Add(kvazaar 
  GIT_REPOSITORY https://github.com/ultravideo/kvazaar.git
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/kvazaar-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/kvazaar-build
  TEST_COMMAND      ""
  CMAKE_ARGS        -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
)

ExternalProject_Add(libde265
  GIT_REPOSITORY https://github.com/strukturag/libde265.git
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/libde265-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/libde265-build
  TEST_COMMAND      ""
  CMAKE_ARGS        -DENABLE_SDL=OFF  -DENABLE_DECODER=OFF -DBUILD_SHARED_LIBS=OFF ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
)

ExternalProject_Add(libwebp
  GIT_REPOSITORY  https://chromium.googlesource.com/webm/libwebp.git
  GIT_TAG         v1.5.0
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/libwebp-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/libwebp-build
  TEST_COMMAND      ""
  CMAKE_ARGS        -DWEBP_LINK_STATIC=ON -DBUILD_SHARED_LIBS=OFF -DWEBP_ENABLE_SIMD=ON
  -DWEBP_ENABLE_SIMD=ON -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF -DWEBP_BUILD_GIF2WEBP=OFF
  -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_LIBWEBPMUX=OFF
  -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF -DWEBP_USE_THREAD=OFF -DWEBP_BUILD_ANIM_UTILS=OFF
      ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
)

# 

# ExternalProject_Add(x265
#   URL http://ftp.videolan.org/pub/videolan/x265/x265_4.1.tar.gz
#   URL_HASH SHA256=a31699c6a89806b74b0151e5e6a7df65de4b49050482fe5ebf8a4379d7af8f29
#   SOURCE_DIR        ${CMAKE_BINARY_DIR}/x265-src
#   BINARY_DIR        ${CMAKE_BINARY_DIR}/x265-build
#   SOURCE_SUBDIR     source
#   TEST_COMMAND      ""
#   CMAKE_ARGS        -DENABLE_LIBNUMA=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.1 -DENABLE_SHARED=OFF -DENABLE_TESTS=OFF -DENABLE_CLI=OFF ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
# )

if (EMSCRIPTEN AND OFF)
    ExternalProject_Add(zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    SOURCE_DIR        ${CMAKE_BINARY_DIR}/zlib-src
    BINARY_DIR        ${CMAKE_BINARY_DIR}/zlib-build
    TEST_COMMAND      ""
    CMAKE_ARGS        -DZLIB_BUILD_TESTING=OFF -DZLIB_BUILD_SHARED=OFF -DZLIB_BUILD_STATIC=ON ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
  )
  ExternalProject_Add(png
    GIT_REPOSITORY https://github.com/pnggroup/libpng.git
    SOURCE_DIR        ${CMAKE_BINARY_DIR}/libpng-src
    BINARY_DIR        ${CMAKE_BINARY_DIR}/libpng-build
    TEST_COMMAND      ""
    CMAKE_ARGS        -DPNG_TESTS=OFF -DPNG_EXECUTABLES=OFF -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_DEBUG=OFF -DZLIB_INCLUDE_DIR=${CMAKE_BINARY_DIR}/zlib-build/install/install/include -DZLIB_LIBRARY=${CMAKE_BINARY_DIR}/zlib-build/install/install/lib/libz.a ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
    DEPENDS           zlib
  )
  ExternalProject_Add(aom
    GIT_REPOSITORY https://aomedia.googlesource.com/aom
    GIT_TAG           v3.12.1
    SOURCE_DIR        ${CMAKE_BINARY_DIR}/aom-src
    BINARY_DIR        ${CMAKE_BINARY_DIR}/aom-build
    TEST_COMMAND      ""
    CMAKE_ARGS        -DAOM_TARGET_CPU=generic -DBUILD_SHARED_LIBS=OFF -DCONFIG_AV1_ENCODER=1 -DENABLE_DOCS=0 -DENABLE_EXAMPLES=0 -DENABLE_TESTDATA=0 -DENABLE_TESTS=0 -DENABLE_TOOLS=0 -DENABLE_CCACHE=1 ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
  )
endif()



# set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/aom/bui")

set(LIBDE265_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libde265-build/install/include)
set(LIBDE265_LIBRARY_DIR ${CMAKE_BINARY_DIR}/libde265-build/libde265)
set(LIBDE265_LIBRARY ${LIBDE265_LIBRARY_DIR}/libde265${CMAKE_STATIC_LIBRARY_SUFFIX})
set(KVAZAAR_INCLUDE_DIR ${CMAKE_BINARY_DIR}/kvazaar-build/install/include)
set(KVAZAAR_LIBRARY_DIR ${CMAKE_BINARY_DIR}/kvazaar-build/install/lib)
set(KVAZAAR_LIBRARY ${KVAZAAR_LIBRARY_DIR}/libkvazaar${CMAKE_STATIC_LIBRARY_SUFFIX})
set(LIBHEIF_LIBRARY_DIR ${CMAKE_BINARY_DIR}/libheif-build/install/lib)
set(LIBSHARPYUV_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libwebp-build/install/include/webp)
set(LIBSHARPYUV_LIBRARY_DIR ${CMAKE_BINARY_DIR}/libwebp-build/install/lib)
set(LIBSHARPYUV_LIBRARY ${LIBSHARPYUV_LIBRARY_DIR}/libsharpyuv${CMAKE_STATIC_LIBRARY_SUFFIX})

set(EXTRA_LIB_ARGS -DLIBDE265_INCLUDE_DIR=${LIBDE265_INCLUDE_DIR})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DLIBDE265_LIBRARY=${LIBDE265_LIBRARY})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DKVAZAAR_INCLUDE_DIR=${KVAZAAR_INCLUDE_DIR})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DKVAZAAR_LIBRARY=${KVAZAAR_LIBRARY})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DLIBDE265_LIBRARY=${LIBDE265_LIBRARY})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DKVAZAAR_INCLUDE_DIR=${KVAZAAR_INCLUDE_DIR})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DKVAZAAR_LIBRARY=${KVAZAAR_LIBRARY})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DLIBSHARPYUV_INCLUDE_DIR=${LIBSHARPYUV_INCLUDE_DIR})
set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DLIBSHARPYUV_LIBRARY=${LIBSHARPYUV_LIBRARY})

if (EMSCRIPTEN AND EMS_EXTRA)
  set(AOM_INCLUDE_DIR ${CMAKE_BINARY_DIR}/aom-build/install/include)
  set(AOM_LIBRARY_DIR ${CMAKE_BINARY_DIR}/aom-build/install/lib)
  set(AOM_LIBRARY ${AOM_LIBRARY_DIR}/libaom${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DZLIB_LIBRARY=${CMAKE_BINARY_DIR}/zlib-build/install/install/lib/libz${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DZLIB_INCLUDE_DIR=${CMAKE_BINARY_DIR}/zlib-build/install/install/include)
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DPNG_PNG_INCLUDE_DIR=${CMAKE_BINARY_DIR}/libpng-build/install/include)
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DPNG_LIBRARY=${CMAKE_BINARY_DIR}/libpng-build/install/lib/libpng${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DAOM_INCLUDE_DIR=${AOM_INCLUDE_DIR})
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DAOM_LIBRARY=${AOM_LIBRARY})

  set(CORE_LINKERS -laom -L${AOM_LIBRARY_DIR})
else()
  set(EXTRA_LIB_ARGS ${EXTRA_LIB_ARGS} -DWITH_AOM_ENCODER=OFF -DWITH_AOM_DECODER=OFF)
  set(CORE_LINKERS )
endif()
# 00c8796cd0df98d9724577cf9110093d3915d549 <-> 5e9deb19fe6b3768af0bb8e9e5e8438b15171bf3
ExternalProject_Add(
  libheif
  GIT_TAG           e0bfb132ab984ad3d7703b5183a9fb336d96f8a0
  GIT_REPOSITORY    https://github.com/strukturag/libheif.git
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/libheif-src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/libheif-build
  TEST_COMMAND      ""
  CMAKE_ARGS        -DWITH_LIBSHARPYUV=ON ${EXTRA_LIB_ARGS} -DENABLE_MULTITHREADING_SUPPORT=OFF -DWITH_SvtEnc=OFF -DWITH_RAV1E=OFF -DENABLE_EXPERIMENTAL_FEATURES=OFF -DWITH_KVAZAAR=ON -DWITH_X265=OFF -DWITH_GDK_PIXBUF=OFF -DWITH_OpenH264_DECODER=OFF -DWITH_EXAMPLES=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF ${EXTERNAL_PROJECT_EXTRA_CMAKE_ARGS}
  DEPENDS           libde265 kvazaar libwebp
)

set(CMAKE_CXX_STANDARD 17)

add_library(elheif STATIC src/impl.cpp)
add_dependencies(elheif libheif)

target_include_directories(elheif PUBLIC include)
target_include_directories(elheif PUBLIC ${CMAKE_BINARY_DIR}/libheif-build/install/include)

if(MSVC)
  # Ensure paths to static libraries are correctly used
  target_link_libraries(elheif PUBLIC
    ${LIBHEIF_LIBRARY}
    ${LIBDE265_LIBRARY}
    ${KVAZAAR_LIBRARY}
    ${LIBSHARPYUV_LIBRARY}
    ${CORE_LINKERS}
  )
else()
  # Use the Unix-like flags for Linux/macOS
  target_link_libraries(elheif PUBLIC
    # -pthread
    -lheif
    -lde265
    -lkvazaar
    -lsharpyuv
    -L${KVAZAAR_LIBRARY_DIR}
    -L${LIBDE265_LIBRARY_DIR}
    -L${LIBHEIF_LIBRARY_DIR}
    -L${LIBSHARPYUV_LIBRARY_DIR}
    ${CORE_LINKERS}
  )
endif()
