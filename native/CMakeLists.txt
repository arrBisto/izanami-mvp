cmake_minimum_required(VERSION 3.22.1)
project(izanami-native)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -mcpu=cortex-a78 -ffast-math -fno-finite-math-only -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mcpu=cortex-a78 -ffast-math -fno-finite-math-only -fPIC")

add_compile_definitions(PLATFORM_ANDROID)
set(ANDROID_NDK ${CMAKE_SOURCE_DIR}/ndk)

add_compile_definitions(GGML_VERSION="1.0.0")
add_compile_definitions(GGML_COMMIT="master")

include_directories(
    /data/data/com.termux/files/usr/include
    ${CMAKE_SOURCE_DIR}/ndk/sysroot/usr/include
    ${CMAKE_SOURCE_DIR}/ndk/sources/android/native_app_glue
    /data/data/com.termux/files/usr/include/luajit-2.1
)

set(LLAMA_CPP_DIR ${CMAKE_SOURCE_DIR}/../submodules/llama.cpp)
include_directories(${LLAMA_CPP_DIR}/ggml/src)
include_directories(${LLAMA_CPP_DIR}/ggml/src/ggml-cpu)
include_directories(${LLAMA_CPP_DIR}/include)

set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "Build server" FORCE)

add_subdirectory(raylib)
add_subdirectory(${LLAMA_CPP_DIR} ${CMAKE_BINARY_DIR}/llama_cpp_build)
add_subdirectory(${LLAMA_CPP_DIR}/tools/mtmd ${CMAKE_BINARY_DIR}/mtmd_build)

add_library(main SHARED
    main.cpp
    InferenceEngine.cpp
    ChatEngine.cpp
    AndroidInput.cpp
    Keypad.cpp
    LuaEngine.cpp
    PackageManager.cpp
    CrashGuard.cpp
    MemoryVault.cpp
    Forge.cpp
    TextWrap.cpp
    Attach.cpp
    ModelRouter.cpp
    PatchExecutor.cpp
)

target_link_libraries(main
    -Wl,--whole-archive
    raylib
    -Wl,--no-whole-archive
    log
    android
    EGL
    GLESv2
    llama
    ggml
    mtmd
    luajit-5.1
    curl
    sqlite3
)
target_link_options(main PRIVATE "-Wl,--allow-multiple-definition")
