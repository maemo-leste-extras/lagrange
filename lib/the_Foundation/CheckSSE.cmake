set (SSE41_FOUND NO)
if (DEFINED TFDN_ENABLE_SSE41 AND NOT TFDN_ENABLE_SSE41)
    return ()
endif ()

try_run (
    sseRunCode
    sseCompiled
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/ssecheck.c
    RUN_OUTPUT_VARIABLE sseOutput
)
string (STRIP "${sseOutput}" sseOutput)
# message (STATUS "sseCompiled: ${sseCompiled}")
# message (STATUS "sseRunCode: ${sseRunCode}")
# message (STATUS "sseOutput: ${sseOutput}")

if (sseCompiled AND sseOutput STREQUAL "1")
    set (SSE41_FOUND YES)
endif ()

if (SSE41_FOUND)
    message (STATUS "CPU supports SSE 4.1")
else ()
    message (STATUS "CPU does not support SSE 4.1")
endif ()
