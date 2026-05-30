# cmake/SyncWasm.cmake
#
# Forces the OS to flush a file's write-back cache to physical storage.
# Called as a POST_BUILD script by loom_add_executable() on WASM targets.
#
# On external drives (USB, Thunderbolt SSDs) the kernel may cache writes
# in memory and not immediately commit them to the device.  Any subsequent
# read that bypasses the page cache (e.g. a new process, after eviction)
# will see zeros.  Opening the file with O_SYNC / calling fsync() drains
# the buffer and ensures the on-disk bytes match what the linker wrote.
#
# Usage (cmake -P):
#   cmake -DWASM_FILE=<path> -P cmake/SyncWasm.cmake

if(NOT DEFINED WASM_FILE)
    message(FATAL_ERROR "SyncWasm.cmake: WASM_FILE not set")
endif()

# Python is available on all supported platforms (macOS ships it; Linux and
# CI environments install it).  We use it for a portable fsync() call.
find_program(_python NAMES python3 python REQUIRED)

execute_process(
    COMMAND "${_python}" -c
        "import os, sys; fd=os.open(sys.argv[1],os.O_RDWR); os.fsync(fd); os.close(fd); print('fsync OK')"
        "${WASM_FILE}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE  _err
)

if(NOT _rc EQUAL 0)
    message(WARNING "SyncWasm: fsync failed for ${WASM_FILE}: ${_err}")
else()
    message(STATUS "Synced: ${WASM_FILE}")
endif()
