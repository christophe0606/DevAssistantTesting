cmake_minimum_required(VERSION 3.25)
project(SdfModelTools NONE)
find_program(UV_EXECUTABLE uv REQUIRED)

# Firmware is built by CMSIS Solution from the csolution/cproject and pack
# components. These targets cover the Python MLOps step and host checks.
# No second ExecuTorch checkout or CMake build of its runtime is needed.
add_custom_target(check_model
    COMMAND ${UV_EXECUTABLE} run --active --no-project python scripts/check_environment.py
    COMMAND ${UV_EXECUTABLE} run --active --no-project python tests/test_tiles.py
    COMMAND ${UV_EXECUTABLE} run --active --no-project python tests/test_export_contract.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL)
add_custom_target(create_ai_layer
    COMMAND ${UV_EXECUTABLE} run --active --no-project python create_ai_layer.py Blinky.cbuild-mlops.yml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL)
