cmake_minimum_required(VERSION 3.16)

project(wren)

add_library(wren
    ../../src/optional/wren_opt_meta.c
    ../../src/optional/wren_opt_random.c
    ../../src/vm/wren_compiler.c
    ../../src/vm/wren_core.c
    ../../src/vm/wren_debug.c
    ../../src/vm/wren_primitive.c
    ../../src/vm/wren_utils.c
    ../../src/vm/wren_value.c
    ../../src/vm/wren_vm.c
)

target_include_directories(wren PUBLIC ../../src/include ../../src/vm ../../src/optional)
target_link_libraries(wren PUBLIC m )