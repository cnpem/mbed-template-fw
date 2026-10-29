string(TIMESTAMP BUILD_TIMESTAMP "%s" UTC)

find_package(Git QUIET)

set(GIT_TAG "UNTAGGED")
set(GIT_HASH "UNKNOWN")
set(GIT_DIRTY FALSE)
set(VERSION "UNKNOWN")

if(GIT_FOUND)
    # Checks whether the current commit has exactly one tag
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --exact-match HEAD
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_TAG
        RESULT_VARIABLE TAG_RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT TAG_RESULT EQUAL 0)
        set(GIT_TAG "UNTAGGED")
    endif()

    # Gets the first 8 characters of the hash
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short=8 HEAD
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_HASH
        RESULT_VARIABLE HASH_RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT HASH_RESULT EQUAL 0)
        set(GIT_HASH "UNKNOWN")
    endif()

    # Check for local changes
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" status --porcelain
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_STATUS
        RESULT_VARIABLE STATUS_RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(STATUS_RESULT EQUAL 0 AND NOT GIT_STATUS STREQUAL "")
        set(GIT_DIRTY TRUE)
    endif()

    # Constructs the version string: [tag-]hash[-dirty]
    if(NOT GIT_TAG STREQUAL "UNTAGGED")
        set(VERSION "${GIT_TAG}-${GIT_HASH}")
    else()
        set(VERSION "${GIT_HASH}")
    endif()

    if(GIT_DIRTY)
        string(APPEND VERSION "-dirty")
    endif()
endif()

file(WRITE "${OUTPUT_FILE}"
"#define BUILD_TIMESTAMP ${BUILD_TIMESTAMP}\n"
"#define VERSION \"${VERSION}\"\n"
"#define GIT_TAG \"${GIT_TAG}\"\n"
"#define GIT_HASH \"${GIT_HASH}\"\n"
"#define GIT_DIRTY ${GIT_DIRTY}\n"
)
