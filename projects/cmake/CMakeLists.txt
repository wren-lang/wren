cmake_minimum_required(VERSION 3.11)
project (Wren
    VERSION 0.4.0
    DESCRIPTION "Wren: A small, fast, class-based concurrent scripting language"
    HOMEPAGE_URL "https://github.com/wren-lang/wren"
    LANGUAGES C
)

# Options
option(WREN_META "Wren Meta Class" ON)
option(WREN_RANDOM "Wren Random Class" ON)
option(WREN_BUILD_STATIC "Wren Static Library" ON)
option(WREN_BUILD_SHARED "Wren Shared Library" OFF)
option(WREN_BUILD_TEST "Build Wren Tests" OFF)

# Source Files
set(WREN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(WREN_SRC_FILES
    ${WREN_SRC}/vm/wren_compiler.c
    ${WREN_SRC}/vm/wren_core.c
    ${WREN_SRC}/vm/wren_debug.c
    ${WREN_SRC}/vm/wren_primitive.c
    ${WREN_SRC}/vm/wren_utils.c
    ${WREN_SRC}/vm/wren_value.c
    ${WREN_SRC}/vm/wren_vm.c
    ${WREN_SRC}/optional/wren_opt_meta.c
    ${WREN_SRC}/optional/wren_opt_random.c
)
set(WREN_INCLUDE_DIRS
    ${WREN_SRC}/include
    ${WREN_SRC}/optional
    ${WREN_SRC}/vm
)

# Defines
set(WREN_DEFINES)
if (WREN_META)
    list(APPEND WREN_DEFINES WREN_OPT_META)
endif()
if (WREN_RANDOM)
    list(APPEND WREN_DEFINES WREN_OPT_RANDOM)
endif()

# Wren Libraries
if (WREN_BUILD_STATIC)
    add_library(wren STATIC ${WREN_SRC_FILES})
    target_include_directories(wren PUBLIC ${WREN_INCLUDE_DIRS})
    target_compile_definitions(wren PUBLIC ${WREN_DEFINES})
endif()
if (WREN_BUILD_SHARED)
    add_library(wren_shared SHARED ${WREN_SRC_FILES})
    target_include_directories(wren_shared PUBLIC ${WREN_INCLUDE_DIRS})
    target_compile_definitions(wren_shared PUBLIC ${WREN_DEFINES})
endif()

# Testing
include(CTest)
enable_testing()
if(WREN_BUILD_TEST AND WREN_BUILD_STATIC)
    set(CTEST_CUSTOM_TESTS_IGNORE pkg-config--static)

    set(WREN_TEST ${WREN_SRC}/../test)
    file(GLOB WREN_TEST_FILES ${WREN_TEST}/api/*.c)
    list(APPEND WREN_TEST_FILES
        ${WREN_TEST}/main.c
        ${WREN_TEST}/test.c
    )

    add_executable(wren_test ${WREN_TEST_FILES})
    target_link_libraries(wren_test PUBLIC wren m)

    add_test(NAME wren_test COMMAND wren_test ${WREN_TEST}/core/system/print.wren)
endif()