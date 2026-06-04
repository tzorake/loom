set(CMAKE_SYSTEM_NAME WASI)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

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
            set(${out_bin} "${_candidate}" PARENT_SCOPE)
            cmake_path(GET _candidate PARENT_PATH _root)
            set(${out_root} "${_root}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    message(FATAL_ERROR
        "WASI SDK not found.  Install it from "
        "https://github.com/WebAssembly/wasi-sdk/releases "
        "then set the WASI_SDK_PATH environment variable to its root directory.")
endfunction()

_loom_find_wasi_bin(WASI_BIN WASI_ROOT)
message(STATUS "Loom: WASI SDK root = ${WASI_ROOT}")
message(STATUS "Loom: WASI SDK bin = ${WASI_BIN}")

set(CMAKE_C_COMPILER "${WASI_BIN}/wasm32-wasi-clang" CACHE PATH "C compiler")
set(CMAKE_CXX_COMPILER "${WASI_BIN}/wasm32-wasi-clang++" CACHE PATH "C++ compiler")
set(CMAKE_AR "${WASI_BIN}/llvm-ar" CACHE PATH "Archiver")
set(CMAKE_RANLIB "${WASI_BIN}/llvm-ranlib" CACHE PATH "Ranlib")
set(CMAKE_SYSROOT "${WASI_ROOT}/share/wasi-sysroot" CACHE PATH "Sysroot")

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-Wl,--no-entry -Wl,--export-dynamic -Wl,--export=memory -Wl,--allow-undefined")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH "${WASI_ROOT}/share/wasi-sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
