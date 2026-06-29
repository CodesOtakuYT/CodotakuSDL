# CodotakuSDL — Agent Notes

## Build

```powershell
cmake --preset default   # or: cmake -B build
cmake --build build
```

## Run Examples

```powershell
./build/triangle
```

## Conventions

- Namespace: `codotaku`
- C++26 (MSVC: `/std:c++latest`)
- No getters/setters on simple data structs
- SDL pointers are raw (centralized ownership in `Runtime`)
- Exceptions for init/GPU failures; `std::expected` for future IO/loading
- GLM for all math types
- SDL logging only no fmt/std::cout/etc.
