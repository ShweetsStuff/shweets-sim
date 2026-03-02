if(NOT DEFINED BIN_DIR OR BIN_DIR STREQUAL "")
    message(FATAL_ERROR "BIN_DIR is required")
endif()

if(NOT DEFINED EXE_FILE OR EXE_FILE STREQUAL "")
    message(FATAL_ERROR "EXE_FILE is required")
endif()

if(NOT DEFINED PACKAGE_NAME OR PACKAGE_NAME STREQUAL "")
    set(PACKAGE_NAME "snivy-win32")
endif()

set(EXE_PATH "${BIN_DIR}/${EXE_FILE}")
set(PACKAGE_DIR "${BIN_DIR}/${PACKAGE_NAME}")
set(ARCHIVE_PATH "${BIN_DIR}/${PACKAGE_NAME}.zip")

file(REMOVE_RECURSE "${PACKAGE_DIR}")
file(REMOVE "${ARCHIVE_PATH}")
file(MAKE_DIRECTORY "${PACKAGE_DIR}")

if(EXISTS "${EXE_PATH}")
    file(COPY "${EXE_PATH}" DESTINATION "${PACKAGE_DIR}")
else()
    message(FATAL_ERROR "Executable not found: ${EXE_PATH}")
endif()

if(EXISTS "${BIN_DIR}/resources")
    file(COPY "${BIN_DIR}/resources" DESTINATION "${PACKAGE_DIR}")
endif()

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar cf "${PACKAGE_NAME}.zip" --format=zip "${PACKAGE_NAME}"
        WORKING_DIRECTORY "${BIN_DIR}"
        RESULT_VARIABLE ZIP_RESULT
)
if(NOT ZIP_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed creating ${ARCHIVE_PATH}")
endif()
