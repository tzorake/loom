# cmake/toolchains/wasm32.cmake
#
# Cross-compilation toolchain for WebAssembly using the WASI SDK.
#
# Usage:
#   cmake -B build-wasm \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/wasm32.cmake \
#         -DLOOM_BUILD_TESTS=OFF
#
# The WASI SDK can be downloaded from:
#   https://github.com/WebAssembly/wasi-sdk/releases
#
# Discovery order for the SDK:
#   1. WASI_SDK_PATH environment variable  (may point to root OR root/bin)
#   2. /opt/wasi-sdk
#   3. ~/wasi-sdk

cmake_minimum_required(VERSION 3.25)

set(CMAKE_SYSTEM_NAME    WASI)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

# ── Locate the WASI SDK bin directory ─────────────────────────────────────
# Normalise: the env var may point to the root or to root/bin.
function(_loom_find_wasi_bin out_bin out_root)
    set(_candidates "")

    if(DEFINED ENV{WASI_SDK_PATH})
        list(APPEND _candidates "$ENV{WASI_SDK_PATH}")
        list(APPEND _candidates "$ENV{WASI_SDK_PATH}/bin")
    endif()
    list(APPEND _candidates "/opt/wasi-sdk/bin")
    list(APPEND _candidates "$ENV{HOME}/wasi-sdk/bin")

    foreach(_candidate IN LISTS _candidates)
        if(EXISTS "${_candidate}/wasm32-wasi-clang")
            set(${out_bin}  "${_candidate}" PARENT_SCOPE)
            cmake_path(GET _candidate PARENT_PATH _root)
            set(${out_root} "${_root}"    PARENT_SCOPE)
            return()
        endif()
    endforeach()

    message(FATAL_ERROR
        "WASI SDK not found.  Install it from "
        "https://github.com/WebAssembly/wasi-sdk/releases "
        "then set the WASI_SDK_PATH environment variable to its root directory.")
endfunction()

_loom_find_wasi_bin(WASI_BIN WASI_ROOT)
message(STATUS "Loom: WASI SDK root  = ${WASI_ROOT}")
message(STATUS "Loom: WASI SDK bin   = ${WASI_BIN}")

# ── Compilers ──────────────────────────────────────────────────────────────
# Use the wasm32-wasi-clang wrapper scripts; they automatically set the
# correct target triple, sysroot, and default libc++ include paths.
set(CMAKE_C_COMPILER   "${WASI_BIN}/wasm32-wasi-clang"   CACHE PATH "C compiler")
set(CMAKE_CXX_COMPILER "${WASI_BIN}/wasm32-wasi-clang++" CACHE PATH "C++ compiler")
set(CMAKE_AR           "${WASI_BIN}/llvm-ar"             CACHE PATH "Archiver")
set(CMAKE_RANLIB       "${WASI_BIN}/llvm-ranlib"         CACHE PATH "Ranlib")
set(CMAKE_SYSROOT      "${WASI_ROOT}/share/wasi-sysroot" CACHE PATH "Sysroot")

# ── Linker flags ───────────────────────────────────────────────────────────
# --no-entry       : no _start / main entry point; JS calls loom_init()
# --export-dynamic : honour __attribute__((export_name(...))) annotations
# --export=memory  : JS needs access to the linear memory (pixel buffers, strings)
# --allow-undefined: JS-imported functions are resolved at runtime
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-Wl,--no-entry -Wl,--export-dynamic -Wl,--export=memory -Wl,--allow-undefined")

# ── CMake internals ────────────────────────────────────────────────────────
# Prevent CMake from trying to run test executables during configuration.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Constrain find_* to the sysroot so host libraries are never picked up.
set(CMAKE_FIND_ROOT_PATH "${WASI_ROOT}/share/wasi-sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
