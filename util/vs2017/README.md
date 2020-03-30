# Building on windows

Building the `wren` project requires python2 to available in the path as `python`, in order to download and build the libuv dependency. The download and build of libuv is done as a pre-build step by the solution.

You can download and install `wren` using the [vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:

    $ git clone https://github.com/Microsoft/vcpkg.git
    $ cd vcpkg
    $ ./bootstrap-vcpkg.sh
    $ ./vcpkg integrate install
    $ vcpkg install wren

The `wren` port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

