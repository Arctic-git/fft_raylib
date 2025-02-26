# fft_raylib

## Dependencies
- **CMake** (version 3.30 or above)
- **C++23** or higher

## Cloning the Repository
After cloning, apply the following patches for external dependencies.
These patches apply minor fixes.

```bash
cd external

cd imgui
git apply ../imgui.patch
cd ..

cd rlImGui
git apply ../rlImGui.patch
cd ..

cd raylib
git apply ../raylib.patch
cd ..

cd portaudio
git apply ../portaudio.patch
cd ..
```

## Building the Project

### macOS
#### Prerequisites:
Make sure you have **CMake** installed:
```bash
brew install cmake
```

#### Build using CMake:
```bash
mkdir build && cd build
cmake ..
make -j8
```

### Windows
More infos are in the official [raylib Windows setup guide](https://github.com/raysan5/raylib/wiki/Working-on-Windows).

#### MinGW-W64/GCC:
1. Install MinGW-W64/GCC `w64devkit-x64-2.0.0` and add the `bin` folder to your PATH. For example:
    ```bash
    setx PATH "%PATH%;C:\Users\...\bin\"
    ```
2. Install CMake and add its `bin` folder to your PATH.
3. Build with CMake:
    ```bash
    mkdir build; cd build
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    # or
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER:FILEPATH=C:\Users\...\w64devkit\bin\gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:\...\w64devkit\bin\g++.exe ..
    make -j8
    ```

#### Visual Studio:
Todo

#### Troubleshooting:
If the CMake configuration fails the first time, try running the `cmake` command again. Or add Windows defender exception to this folder