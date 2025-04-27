# bin2header
 A tool to create a header file out of a file
- Place compiled executable reachable from cmd/powershell/terminal
- `bin2header my_file.wav`

## Build
```
git clone https://github.com/nezvers/bin2header.git
cd bin2header
```

##### Visual Studio
- Open folder with `Visual Studio`
- Wait until `Output` tab says `CMake generation finished`
- In dropdown menu next to "Play" button choose `bin2header.exe`

##### Cmake
- Cmake should be installed and accessible from cmd/powershell/terminal
```
cmake -S . -B ./build -G "${YOUR_BUILD_TARGET_CHOICE}"
```

###### Example - Windows Mingw with optional debug build
- Mingw64 should be installed and `mingw64/bin` folder added to environment `PATH` variable
```
cmake -S . -B ./build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
mingw32-make -C ./build
./build/bin2header.exe
```