if(NOT DEFINED BIN_DIR OR BIN_DIR STREQUAL "")
    message(FATAL_ERROR "BIN_DIR is required")
endif()

set(ARCHIVE_PATH "${BIN_DIR}/shweets-sim-web.zip")
file(REMOVE "${ARCHIVE_PATH}")

file(GLOB INDEX_OUTPUTS "${BIN_DIR}/index.*")
set(FILES_TO_ZIP "")
foreach(FILE_PATH IN LISTS INDEX_OUTPUTS)
    if(NOT FILE_PATH STREQUAL ARCHIVE_PATH)
        get_filename_component(FILE_NAME "${FILE_PATH}" NAME)
        list(APPEND FILES_TO_ZIP "${FILE_NAME}")
    endif()
endforeach()

if(FILES_TO_ZIP)
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar cf "shweets-sim-web.zip" --format=zip ${FILES_TO_ZIP}
            WORKING_DIRECTORY "${BIN_DIR}"
            RESULT_VARIABLE ZIP_RESULT
    )
    if(NOT ZIP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed creating ${ARCHIVE_PATH}")
    endif()
else()
    message(WARNING "No index.* files found in ${BIN_DIR}; skipping shweets-sim-web.zip creation")
endif()
