# For Linux:

1. cd into the root folder(TexWelder)
2. run this everytime you build: `rm -rf build && cmake -B build && cd build && make && cd .. && ./build/TexWelder`


# For Windows:

1. cd into the root folder(TexWelder)
2. run this everytime you build (in cmd.exe): `if exist build rmdir /s /q build && cmake -B build && cmake --build build --config Release && build\Release\TexWelder.exe`

Notes:
- If CMake picked a single config generator instead (like MinGW Makefiles, Ninja), the executable ends up at `build\TexWelder.exe` instead of `build\Release\TexWelder.exe` so adjust the last part of the command accordingly


# For Windows (cross-compiled from Linux):

1. install the mingw-w64 toolchain: `sudo dnf install mingw64-gcc mingw64-gcc-c++ mingw64-winpthreads-static` (if your distro's repo 404s on these, grab the rpms from fedoras mirror and `sudo dnf install` them locally)
   - dnf is fedora family only (fedora, nobara etc.), other distros need different packages:
     - debian/ubuntu: `sudo apt install mingw-w64`
     - arch: `sudo pacman -S mingw-w64-gcc`
     - opensuse: `sudo zypper install mingw64-cross-gcc-c++ mingw64-cross-gcc mingw64-winpthreads-devel`
2. cd into the root folder(TexWelder)
3. run this everytime you build: `rm -rf build-windows && cmake -B build-windows -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release && cmake --build build-windows -j$(nproc)`
4. the exe ends up at `build-windows/TexWelder.exe`, you can test it with wine: `wine build-windows/TexWelder.exe`



## extra notes:
- you must have cmake installed, and a C++ compiler toolchain (Visual Studio Build Tools on Windows, or gcc/make on Linux).
