if(NOT DEFINED WASM_FILE)
    message(FATAL_ERROR "CheckWasm.cmake: WASM_FILE not set")
endif()

if(NOT EXISTS "${WASM_FILE}")
    message(FATAL_ERROR "CheckWasm.cmake: output file does not exist: ${WASM_FILE}")
endif()

find_program(_python NAMES python3 python REQUIRED)

execute_process(
    COMMAND "${_python}" -c "
import sys, os

path = sys.argv[1]
size = os.path.getsize(path)

with open(path, 'rb') as f:
    data = f.read()

# 1. Magic bytes
if data[:4] != b'\\x00asm':
    print(f'ERROR: bad magic {data[:4].hex()} (expected 0061736d)')
    sys.exit(1)

# 2. Minimum plausible size for a non-trivial WASM module
if size < 64 * 1024:
    print(f'ERROR: file too small ({size} bytes); linker may have failed')
    sys.exit(2)

# 3. Scan for zero runs longer than 4 KB anywhere in the file.
#    Legitimate WASM sections can contain short zero spans (e.g. alignment
#    padding), but a multi-KB run of zeros always indicates a partial write.
ZERO_RUN_LIMIT = 4096
zero_run = 0
max_zero_run = 0
for byte in data:
    if byte == 0:
        zero_run += 1
        if zero_run > max_zero_run:
            max_zero_run = zero_run
        if zero_run > ZERO_RUN_LIMIT:
            print(f'ERROR: zero run of >{ZERO_RUN_LIMIT} bytes detected — partial write (max run={max_zero_run})')
            sys.exit(3)
    else:
        zero_run = 0

print(f'WASM OK: {size} bytes, max zero run={max_zero_run}')
"   "${WASM_FILE}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE  _err
)

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "WASM output is corrupt: ${_out}${_err}\n"
        "File: ${WASM_FILE}\n"
        "Delete the output file and rebuild:\n"
        "  cmake -E remove \"${WASM_FILE}\"\n"
        "  cmake --build <build-dir> --target <target>")
endif()

message(STATUS "${_out}")
