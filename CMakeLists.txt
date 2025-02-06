# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================




cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
project(ws2812 C CXX ASM)
pico_sdk_init()
add_executable(ws2812 ws2812.c inc/ssd1306.c)

pico_set_program_name(ws2812 "ws2812")
pico_set_program_version(ws2812 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(ws2812 0)
pico_enable_stdio_usb(ws2812 1)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
target_sources(ws2812 PRIVATE ws2812.c)
target_link_libraries(ws2812 PRIVATE pico_stdlib hardware_pio hardware_i2c)

# Add the standard include files to the build
target_include_directories(ws2812 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  )

pico_add_extra_outputs(ws2812) 