string(TIMESTAMP BUILD_TIMESTAMP "%m/%d/%Y %H:%M:%S")

find_package(Git QUIET)

set(VERSION "UNKNOWN")

if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --exact-match HEAD
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE TAG_RESULT
    )

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short=8 HEAD
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE HASH_RESULT
    )

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" status --porcelain
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_STATUS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE STATUS_RESULT
    )

    if(HASH_RESULT EQUAL 0)
        if(NOT TAG_RESULT EQUAL 0)
            set(GIT_TAG "")
        endif()

        set(VERSION "${GIT_TAG}${GIT_HASH}")

        if(GIT_TAG)
            set(VERSION "${GIT_TAG}:${GIT_HASH}")
        endif()

        if(STATUS_RESULT EQUAL 0 AND GIT_STATUS)
            string(APPEND VERSION "-dirty")
        endif()
    endif()
endif()

file(WRITE "${OUTPUT_FILE}"
    "#define BUILD_TIMESTAMP \"${BUILD_TIMESTAMP}\"\n"
    "#define VERSION \"${VERSION}\"\n"
)
