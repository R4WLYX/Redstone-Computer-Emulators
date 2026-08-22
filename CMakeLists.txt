cmake_minimum_required(VERSION 3.10.0)
project(EMULATOR VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(emulator ${SOURCES})

target_include_directories(emulator PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/units
)

if(EXISTS ${CMAKE_SOURCE_DIR}/res)
    file(GLOB_RECURSE GAME_RESOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/res/*"
    )

    set(RESOURCE_STAMP "${CMAKE_BINARY_DIR}/resources.stamp")

    add_custom_command(
        OUTPUT ${RESOURCE_STAMP}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/res
                $<TARGET_FILE_DIR:emulator>/res
        COMMAND ${CMAKE_COMMAND} -E touch ${RESOURCE_STAMP}
        DEPENDS ${GAME_RESOURCES}
    )

    add_custom_target(copy_resources ALL
        DEPENDS ${RESOURCE_STAMP}
    )
endif()

target_compile_options(emulator PRIVATE
    $<$<CONFIG:Release>:
        -O3
        -ffast-math
        -march=native
    >
    $<$<CONFIG:RelWithDebInfo>:
        -O2
        -g
    >
)