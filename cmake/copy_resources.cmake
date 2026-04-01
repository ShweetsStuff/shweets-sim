if(NOT DEFINED SRC_DIR OR NOT DEFINED DST_DIR OR NOT DEFINED STAMP_FILE)
    message(FATAL_ERROR "SRC_DIR, DST_DIR, and STAMP_FILE must be defined")
endif()

set(CHARACTERS_DIR "${SRC_DIR}/characters")

file(REMOVE_RECURSE "${DST_DIR}")
file(MAKE_DIRECTORY "${DST_DIR}")

# Copy all resources except character source folders and any stray character zips.
file(COPY "${SRC_DIR}/" DESTINATION "${DST_DIR}"
        PATTERN "characters/*" EXCLUDE
        PATTERN "characters/*.zip" EXCLUDE)

file(MAKE_DIRECTORY "${DST_DIR}/characters")
if(EXISTS "${CHARACTERS_DIR}")
    file(GLOB CHILDREN RELATIVE "${CHARACTERS_DIR}" "${CHARACTERS_DIR}/*")
    foreach(CHILD IN LISTS CHILDREN)
        set(CHARACTER_SOURCE_DIR "${CHARACTERS_DIR}/${CHILD}")
        if(NOT IS_DIRECTORY "${CHARACTER_SOURCE_DIR}")
            continue()
        endif()

        file(GLOB_RECURSE CHARACTER_CONTENTS RELATIVE "${CHARACTER_SOURCE_DIR}" "${CHARACTER_SOURCE_DIR}/*")
        if(NOT CHARACTER_CONTENTS)
            message(STATUS "Skipping empty character directory: ${CHARACTER_SOURCE_DIR}")
            continue()
        endif()

        execute_process(
                COMMAND "${CMAKE_COMMAND}" -E tar cf "${DST_DIR}/characters/${CHILD}.zip" --format=zip ${CHARACTER_CONTENTS}
                WORKING_DIRECTORY "${CHARACTER_SOURCE_DIR}"
                RESULT_VARIABLE ZIP_GENERATE_RESULT
        )
        if(NOT ZIP_GENERATE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed generating ${DST_DIR}/characters/${CHILD}.zip (exit code ${ZIP_GENERATE_RESULT})")
        endif()
    endforeach()
endif()

file(WRITE "${STAMP_FILE}" "resources copied\n")
