# RpcClient unary HTTP (Task 5)

set(_dsh_rpc_sources
  src/services/rpc/RpcClient.cpp
)

target_sources(dsh-desktop PRIVATE ${_dsh_rpc_sources})
target_include_directories(dsh-desktop PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

qt_add_executable(tst_rpc_envelope
  tests/tst_rpc_envelope.cpp
  ${_dsh_rpc_sources}
)

target_include_directories(tst_rpc_envelope PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(tst_rpc_envelope PRIVATE Qt6::Network Qt6::Test)

add_test(NAME tst_rpc_envelope COMMAND tst_rpc_envelope)
