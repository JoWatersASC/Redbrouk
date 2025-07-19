cmake_minimum_required(VERSION 3.22)

enable_testing()

set(TEST_CTX "Pipelined" CACHE STRING "What we're testing" FORCE)
set(TEST_TGT "Server"    CACHE STRING "Server or client side" FORCE)
set(TEST_NAME "${TEST_CTX}-${TEST_TGT}" CACHE STRING "Full test name" FORCE)


add_executable("${TEST_NAME}" ./pl_server.cc)
target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${TEST_NAME} PRIVATE Redbrouk-core)

set(TEST_TGT "Client")
set(TEST_NAME "${TEST_CTX}-${TEST_TGT}" CACHE STRING "Full test name" FORCE)
add_executable(${TEST_NAME} ./pl_client.cc)
target_link_libraries(${TEST_NAME} PRIVATE Redbrouk-core)

set(TEST_CTX "ValType-")
set(TEST_TGT "TSet")
set(TEST_NAME "${TEST_CTX}-${TEST_TGT}" CACHE STRING "Full test name" FORCE)
add_executable(${TEST_NAME} ./tset_test.cc)
target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${TEST_NAME} PRIVATE Redbrouk-core)

set(TEST_CTX "Type")
set(TEST_TGT "RBTree")
set(TEST_NAME "${TEST_CTX}-${TEST_TGT}" CACHE STRING "Full test name" FORCE)
add_executable(${TEST_NAME} ./rbt_test.cc)
target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${TEST_NAME} PRIVATE Redbrouk-core)
#[[
set(TEST_CTX "KV")
set(TEST_TGT "Server")
set(TEST_NAME "${TEST_CTX}-${TEST_TGT}" CACHE STRING "Full test name" FORCE)
add_executable(${TEST_NAME} ./kv_server.cc)
target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${TEST_NAME} PRIVATE Redbrouk-core)

set(TEST_TGT "Client")
set(TEST_NAME "${TEST_CTX}-${TEST_TGT}" CACHE STRING "Full test name" FORCE)
add_executable(${TEST_NAME} ./kv_client.cc)
target_link_libraries(${TEST_NAME} PRIVATE Redbrouk-core)
]]

#[[
add_executable(ConnectionTestServer ./connection_test.cc)
target_include_directories(ConnectionTestServer PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(ConnectionTestServer PRIVATE Redbrouk-core)

add_executable(ConnectionTestClient ./connection_test_c.cc)

add_test(
	NAME connection_test
	COMMAND ConnectionTest
)
]]
