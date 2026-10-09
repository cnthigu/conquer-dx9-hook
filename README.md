# ConquerDX9.Hook

DirectX9 hooking for Conquer Online with ImGui overlay

![Preview](img/img.png)

## Info

- Tested on Conquer Online game versions
- Compile on Release & x86
- Uses MinHook for function hooking
- ImGui overlay (toggle with INSERT key)
- Features: Always Jump, Wireframe/Chams, String modification

## Building

1. Select Release configuration and x86 platform
2. Build the solution
3. Output: `Release/Chat.dll`

## Usage

### Version 6609 (Proxy Method)

1. Rename original `Chat.dll` to `OChat.dll` in the game'folder
2. Copy compiled `Chat.dll` to the same folder
3. Launch the game (no injector needed)
4. Press INSERT to toggle ImGui interface

### Other Versions (DLL Injection)

1. Remove proxy code from `src/hooks/proxy.cpp` and `src/dllmain.cpp`
2. Compile as regular DLL
3. Inject the DLL into the game process
4. Press INSERT to toggle ImGui interface

## Credits

Based on examples and concepts from [co-stuff/posts](https://github.com/co-stuff/posts)

## Libraries

- [MinHook](https://github.com/TsudaKageyu/minhook) (included)
- [ImGui](https://github.com/ocornut/imgui) (included)

