## WinAPI Helpers

### Description

Set of C++ WinAPI wrappers, the following functionality is covered:

* BIOS information
* COM Initializer
* Windows exceptions (deprecated in C++11)
* Remote Access Service exceptions
* RAII wrappers for Windows handles
* RAII wrappers for Windows Heap
* Volume and partition information
* CPU information
* Native memory management
* Single instance application pattern
* Process and elevation management
* Windows Registry management
* Service Control Manager helper
* Special paths (e.g. System directory, Temp directory, Local Appdata directory)
* General system information (e.g. Windows version, build, edition)
* General user information (e.g. Username, GUID, SID, Home directory)

### Build

Generate CMake config files, and run build using the active toolset (MSVC for Windows, Ninja for Linux)

### Windows

```
cmake .. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_VERSION=6.1
cmake --build . --clean-first --config Release --parallel 2 --verbose
```

### Linux

```
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --clean-first --config Release --parallel 2 --verbose
```
