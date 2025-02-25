```
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

## build
### windows
- https://github.com/raysan5/raylib/wiki/Working-on-Windows
- MinGW-W64/GCC
  - install and add bin folder to path https://github.com/skeeto/w64devkit/
    -  `setx PATH "%PATH%;C:\Users\ex6207\AppData\Local\Programs\Git\mingw64\bin\"`
  - install cmake and add bin folder to path
  - `mkdir build; cd build;`
  - `cmake -G "MinGW Makefiles" ..`
  - `make -j8`
  - if cmake configure fails the first time try again