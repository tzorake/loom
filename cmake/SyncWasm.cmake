if(NOT DEFINED WASM_FILE)
    message(FATAL_ERROR "SyncWasm.cmake: WASM_FILE not set")
endif()

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
