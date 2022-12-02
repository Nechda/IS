cmake_minimum_required(VERSION 3.12)
project(sten-wav)

set(COMPILE_SETTINGS -O2 -DNDEBUG)

set(BINARY "encoder")
add_executable(${BINARY} encoder.cpp)
target_compile_features(${BINARY} PRIVATE cxx_std_17)
target_compile_options(${BINARY} PRIVATE ${COMPILE_SETTINGS})

set(BINARY "decoder")
add_executable(${BINARY} decoder.cpp)
target_compile_features(${BINARY} PRIVATE cxx_std_17)
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