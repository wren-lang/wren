# Wren CMake Configuration

Allows building the [Wren scripting language](https://wren.io) with [CMake](https://cmake.org).

## Usage

In your project, add Wren's `cmake` directory, and then target either `wren` or `wren_shared`...

``` cmake
add_subdirectory(path/to/wren/projects/cmake)
target_link_libraries(myproject PUBLIC wren)
```

### Testing

To build Wren locally, and test the CMake build, use the following:

``` sh
mkdir build
cd build
cmake .. -DWREN_BUILD_TEST=ON
make
make test
```