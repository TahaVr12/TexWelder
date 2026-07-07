# For Linux:

1. cd into the root folder(TexWelder)
2. run this everytime you build: `rm -rf build && cmake -B build && cd build && make && cd .. && ./build/TexWelder`

# For Windows:

1. cd into the root folder(TexWelder)
2. run this everytime you build (in cmd.exe): `if exist build rmdir /s /q build && cmake -B build && cmake --build build --config Release && build\Release\TexWelder.exe`

Notes:
- If CMake picked a single config generator instead (like MinGW Makefiles, Ninja), the executable ends up at `build\TexWelder.exe` instead of `build\Release\TexWelder.exe` so adjust the last part of the command accordingly

## extra notes:
- you must have cmake installed, and a C++ compiler toolchain (Visual Studio Build Tools on Windows, or gcc/make on Linux).
