cmake_minimum_required(VERSION 3.12)
project(sten-wav)

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(SOURCES
    main.cpp
    fft.cpp
)

# -Wall -Wextra -Werror
set(COMPILE_SETTINGS_DBG -g -O0 )
set(DBG_BINARY "${PROJECT_NAME}_dbg")

add_executable(${DBG_BINARY} ${SOURCES})
target_compile_features(${DBG_BINARY} PRIVATE cxx_std_14)
target_compile_options(${DBG_BINARY} PRIVATE ${COMPILE_SETTINGS_DBG})

set(COMPILE_SETTINGS -O2 -DNDEBUG)
set(BINARY "${PROJECT_NAME}")

add_executable(${BINARY} ${SOURCES})
target_compile_features(${BINARY} PRIVATE cxx_std_14)
target_compile_options(${BINARY} PRIVATE ${COMPILE_SETTINGS})


find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/*.[ch]pp
        ${PROJECT_SOURCE_DIR}/*.[ch]
    )

    add_custom_target(
        clang-force-format
        COMMAND clang-format
        -i
        -style=file
        ${ALL_CXX_SOURCE_FILES}
    )
endif()