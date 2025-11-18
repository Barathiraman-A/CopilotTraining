# Vehicle Telematics Firmware - CMake Build System
cmake_minimum_required(VERSION 3.20)

# Project configuration
project(telematics-firmware
    VERSION 1.0.0
    LANGUAGES C ASM
    DESCRIPTION "Vehicle Telematics Unit Firmware"
)

# Specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/drivers
    ${CMAKE_SOURCE_DIR}/src/middleware
    ${CMAKE_SOURCE_DIR}/src/hal
    ${CMAKE_SOURCE_DIR}/src/app
)

# Source files - Drivers
file(GLOB DRIVER_SOURCES
    src/drivers/can_driver.c
    src/drivers/adc_driver.c
    src/drivers/gps_driver.c
)

# Source files - Middleware
file(GLOB MIDDLEWARE_SOURCES
    src/middleware/circular_buffer.c
    src/middleware/power_manager.c
)

# Source files - Application
file(GLOB APP_SOURCES
    src/main.c
)

# All source files
set(ALL_SOURCES
    ${DRIVER_SOURCES}
    ${MIDDLEWARE_SOURCES}
    ${APP_SOURCES}
)

# Create executable
add_executable(${PROJECT_NAME}.elf ${ALL_SOURCES})

# Link libraries (FreeRTOS would be added here in real implementation)
# target_link_libraries(${PROJECT_NAME}.elf FreeRTOS)

# Generate additional output formats
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating HEX and BIN files and displaying size information"
)

# Print memory usage
add_custom_target(size
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Displaying memory usage"
)

# Flash target (requires OpenOCD or similar)
add_custom_target(flash
    COMMAND openocd -f interface/stlink.cfg -f target/stm32l4x.cfg -c "program ${PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing firmware to target device"
)

# Clean additional files
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES
    "${PROJECT_NAME}.hex;${PROJECT_NAME}.bin"
)

# Installation
install(TARGETS ${PROJECT_NAME}.elf
    RUNTIME DESTINATION bin
)

install(FILES
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    DESTINATION bin
)

# Print configuration summary
message(STATUS "==================== Configuration Summary ====================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "===============================================================")
