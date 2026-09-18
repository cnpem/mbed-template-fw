cmake_minimum_required(VERSION 3.16)

set(MBED_APP_JSON_PATH mbed_app.json)

include(mbed-os/tools/cmake/app.cmake)
add_subdirectory(mbed-os)

add_subdirectory(app)
add_subdirectory(lib)

set(TARGET_CONTROLLER LPC1768)
add_subdirectory(newboot)

project(mbed-template-fw C CXX ASM)

string(TIMESTAMP BUILD_TIMESTAMP "%s" UTC)

find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_TAG
      ERROR_QUIET
      RESULT_VARIABLE GIT_TAG_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE)

      if(NOT GIT_TAG_RESULT EQUAL 0)
          set(GIT_TAG "UNTAGGED")
      endif()

    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_BRANCH
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --always --dirty
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_HASH_DESCRIBE
      OUTPUT_STRIP_TRAILING_WHITESPACE)

else()
    set(GIT_TAG "UNKNOWN")
    set(GIT_BRANCH "UNKNOWN")
    set(GIT_HASH_DESCRIBE "UNKNOWN")
endif()

include_directories(${APP_HDRS} ${LIB_HDRS} ${MODULES_HDRS})

add_executable(${PROJECT_NAME} ${APP_SRCS} ${LIB_SRCS} ${MODULES_SRCS})

target_compile_definitions(${PROJECT_NAME}
PRIVATE
    BUILD_TIMESTAMP=${BUILD_TIMESTAMP}
    GIT_TAG="${GIT_TAG}"
    GIT_BRANCH="${GIT_BRANCH}"
    GIT_HASH_DESCRIBE="${GIT_HASH_DESCRIBE}")

target_link_libraries(${PROJECT_NAME} mbed-os mbed-netsocket)
mbed_set_post_build(${PROJECT_NAME})

mbed_finalize_build()

add_custom_target(full_binary
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/
  DEPENDS ${PROJECT_NAME} newboot
  # Create a pad file with the total size of bootloader section (0x2000)
  COMMAND dd if=/dev/zero bs=1 count=8192 | tr \"\\000\" \"\\377\" >
            newboot_pad.bin
  # Add padding bytes to the bootloader binary
  COMMAND dd if=newboot/bootloader/newboot.bin of=newboot_pad.bin conv=notrunc
  # Concatenate bootloader and app binaries
  COMMAND cat newboot_pad.bin ${PROJECT_NAME}.bin > ${PROJECT_NAME}_full.bin
)
