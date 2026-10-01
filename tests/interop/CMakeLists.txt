include(${PROJECT_SOURCE_DIR}/cmake/capnzero_generate_cpp.cmake)
capnzero_generate_cpp(
  GEN_CPP_SOURCES generated_sources
  GEN_CPP_HEADERS generated_headers
  IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Interop.toml"
)
add_library(interop-schema STATIC ${generated_sources} ${generated_headers})
target_include_directories(interop-schema PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(interop-schema PUBLIC capnzero)
add_executable(interop-server server.cpp)
target_link_libraries(interop-server PRIVATE interop-schema)
add_executable(interop-client client.cpp)
target_link_libraries(interop-client PRIVATE interop-schema)
add_test(NAME interop-cpp
  COMMAND "${CAPNZERO_TEST_PYTHON}" "${CMAKE_CURRENT_SOURCE_DIR}/test_interop.py"
    --cpp-client "$<TARGET_FILE:interop-client>" --cpp-server "$<TARGET_FILE:interop-server>" --cpp-only)
set_tests_properties(interop-cpp PROPERTIES TIMEOUT 30)
set(CAPNZERO_RUNTIME_PYTHON "" CACHE FILEPATH "Python interpreter with pycapnp and pyzmq for interoperability tests")
if(CAPNZERO_RUNTIME_PYTHON)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/python/Interop_capnzero.py" "${CMAKE_CURRENT_BINARY_DIR}/python/Interop.capnp"
    COMMAND "${CAPNZERO_TEST_PYTHON}" "${PROJECT_SOURCE_DIR}/scripts/capnzeroc.py"
      "--descrfile=${CMAKE_CURRENT_SOURCE_DIR}/Interop.toml"
      "--outdir=${CMAKE_CURRENT_BINARY_DIR}/python" --language=python
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Interop.toml"
      "${PROJECT_SOURCE_DIR}/scripts/python_backend.py"
      "${PROJECT_SOURCE_DIR}/scripts/capnzeroc.py"
      "${PROJECT_SOURCE_DIR}/scripts/model.py"
      "${PROJECT_SOURCE_DIR}/scripts/normalization.py"
      "${PROJECT_SOURCE_DIR}/scripts/parsing.py"
      "${PROJECT_SOURCE_DIR}/scripts/wire.py"
      "${PROJECT_SOURCE_DIR}/scripts/capnp_file.py"
    VERBATIM)
  add_custom_target(interop-python-bindings ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/python/Interop_capnzero.py")
  add_test(NAME python-runtime
    COMMAND "${CAPNZERO_RUNTIME_PYTHON}" "${PROJECT_SOURCE_DIR}/tests/python/test_runtime.py"
      "${CMAKE_CURRENT_BINARY_DIR}/python")
  set_tests_properties(python-runtime PROPERTIES TIMEOUT 15)
  add_test(NAME interop-python
    COMMAND "${CAPNZERO_RUNTIME_PYTHON}" "${CMAKE_CURRENT_SOURCE_DIR}/test_interop.py"
      --generator-python "${CAPNZERO_TEST_PYTHON}"
      --cpp-client "$<TARGET_FILE:interop-client>" --cpp-server "$<TARGET_FILE:interop-server>")
  set_tests_properties(interop-python PROPERTIES TIMEOUT 60)
endif()
