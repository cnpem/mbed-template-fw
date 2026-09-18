cmake_minimum_required(VERSION 3.16)

set(MBED_APP_JSON_PATH mbed_app.json)

include(mbed-os/tools/cmake/app.cmake)
add_subdirectory(mbed-os)

add_subdirectory(app)
add_subdirectory(lib)

set(TARGET_CONTROLLER LPC1768)
add_subdirectory(newboot)

project(mbed-template-fw C CXX ASM)

# File containing version information
set(VERSION_INFO_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/app/version_info.h"
)
# Updates information during every build, without configuring CMake again
add_custom_target(update_build_info ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        "-DOUTPUT_FILE=${VERSION_INFO_HEADER}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/update_version_info.cmake"
    BYPRODUCTS "${VERSION_INFO_HEADER}"
    VERBATIM
)

include_directories(${APP_HDRS} ${LIB_HDRS} ${MODULES_HDRS})

add_executable(${PROJECT_NAME} ${APP_SRCS} ${LIB_SRCS} ${MODULES_SRCS}
"${VERSION_INFO_HEADER}")

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
