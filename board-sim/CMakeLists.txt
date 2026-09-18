cmake_minimum_required(VERSION 3.24)
project(espet-board-sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ESPET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

include(FetchContent)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.11
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2)

add_library(board_sim_hw STATIC
    src/gram.c
    src/imu.c
    fake_idf/src/esp_lcd.c
    fake_idf/src/i2c_master.c
    fake_idf/src/gpio.c
    fake_idf/src/freertos.c
)
target_include_directories(board_sim_hw PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/fake_idf/include"
)

add_library(espet_firmware STATIC
    "${ESPET_ROOT}/firmware/main.c"
)
target_include_directories(espet_firmware PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/fake_idf/include"
)
target_link_libraries(espet_firmware PRIVATE board_sim_hw)

add_executable(espet-board-sim
    src/main.c
)
target_include_directories(espet-board-sim PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(espet-board-sim PRIVATE
    espet_firmware
    board_sim_hw
    SDL2::SDL2
)

if(MSVC)
    target_compile_options(board_sim_hw PRIVATE /W3 /utf-8)
    target_compile_options(espet_firmware PRIVATE /W3 /utf-8)
    target_compile_options(espet-board-sim PRIVATE /W3 /utf-8)
endif()

add_custom_command(TARGET espet-board-sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:espet-board-sim>
    COMMENT "Copy SDL2.dll next to espet-board-sim.exe"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT espet-board-sim)
set_property(TARGET espet-board-sim PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${ESPET_ROOT}")
