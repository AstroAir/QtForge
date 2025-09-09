# Repository Guidelines

## Project Structure & Module Organization

- `src/`: C++ sources (core, managers, communication, lua, remote, python bridge).
- `include/`: Public headers exported by the library.
- `tests/`: C++/Python tests and helpers (e.g., `run_all_tests.py`, `test_qtforge_bindings.py`).
- `examples/`: Minimal usage and integration demos (Python and C++).
- `docs/`: User/developer docs; `mkdocs.yml` config.
- `cmake/`, `CMakeLists.txt`, `CMakePresets.json`: Build system files.
- `scripts/`: Cross‑platform build/test helpers (`build.sh`, `build.bat`, `build.py`).
- `qtforge/`: Python type stubs for bindings.

## Build, Test, and Development Commands

- Configure & build (CMake):
  - `cmake -S . -B build -DQTPLUGIN_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug`
  - `cmake --build build --parallel`
- Run C++ tests (CTest):
  - `ctest --test-dir build --output-on-failure`
- Quick scripts:
  - Windows: `scripts\build.bat` • Unix: `scripts/build.sh`
  - Python helper: `python scripts/build.py`
- Python bindings tests:
  - `python tests/run_all_tests.py` (adds `build/src/python` to `PYTHONPATH` if needed)

## Testing Guidelines

- Frameworks: C++ uses Qt Test + CTest; Python uses `unittest` with optional type checks via `mypy` (`mypy.ini`).
- Locations: place C++ tests under `tests/<module>/...`; Python tests as `tests/test_*.py`.
- Conventions: name tests clearly (e.g., `test_plugin_manager.cpp`, `test_qtforge_bindings.py`). Aim to cover error paths and integration flows.
- Run: `ctest -V` for verbose C++; `python -m unittest` or `python tests/run_all_tests.py` for Python.

## Commit & Pull Request Guidelines

- Commits: follow Conventional Commits where possible (`feat:`, `fix:`, `docs:`, `refactor:`; imperative mood). Examples in `git log` include `feat(ui_plugin): ...` and focused refactors.
- PRs must include: clear description, linked issues, test results, docs/README updates when user‑visible, and screenshots for UI‑related changes.
- Pre‑merge checklist: CMake build green, `ctest` passing, Python test runner passing, format C++ with `clang-format` (see `.clang-format`). Update `CHANGELOG.md` when applicable.

## Security & Configuration Tips

- Ensure Qt 6 and toolchains are discoverable: set `CMAKE_PREFIX_PATH` or `Qt6_DIR`.
- For Python bindings, verify the module path: `PYTHONPATH=build/src/python` when running locally.
- Avoid network/SQL usage in tests unless guarded behind feature flags present in the build.
