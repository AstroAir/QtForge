# Development Guide

## Prerequisites
- Qt 6.0+ (Core required; Network/Widgets optional)
- CMake 3.21+
- C++20 compiler

## Build
```
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j
ctest --output-on-failure
```

## Coding Standards
- Modern C++20, RAII, expected<T,E>
- Thread safety with std::mutex/shared_mutex
- Public API documented with Doxygen-style comments

## Pre-commit
- Run clang-format
- Build and run tests
- Update docs and CHANGELOG

