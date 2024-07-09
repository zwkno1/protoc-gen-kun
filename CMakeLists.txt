cmake_minimum_required(VERSION 3.6)

project(protoc-gen-kun)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_BUILD_TYPE Release)

find_package(Protobuf)
find_package(GTest)
find_package(absl)

include_directories(./ ${GTEST_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}
                    ${Protobuf_INCLUDE_DIRS})
add_executable(protoc-gen-kun main.cpp)
target_link_libraries(
  protoc-gen-kun
  protobuf::libprotobuf
  protobuf::libprotoc
  absl::absl_check
  absl::absl_log
  absl::algorithm
  absl::base
  absl::bind_front
  absl::bits
  absl::btree
  absl::cleanup
  absl::cord
  absl::core_headers
  absl::debugging
  absl::die_if_null
  absl::dynamic_annotations
  absl::flags
  absl::flat_hash_map
  absl::flat_hash_set
  absl::function_ref
  absl::hash
  absl::if_constexpr
  absl::layout
  absl::log_initialize
  absl::log_globals
  absl::log_severity
  absl::memory
  absl::node_hash_map
  absl::node_hash_set
  absl::optional
  absl::random_distributions
  absl::random_random
  absl::span
  absl::status
  absl::statusor
  absl::strings
  absl::synchronization
  absl::time
  absl::type_traits
  absl::utility
  absl::variant)

if(WITH_TEST)
  # set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/a.proto") file(GLOB PROTO_FILES
  # "${PROTO_PATH}/*.proto") foreach(PROTO_FILE in ${PROTO_FILES}) string(REGEX
  # REPLACE "[.]proto$" ".pb.cc" OUTPUT_SOURCE ${PROTO_FILE}) list(APPEND
  # OUTPUT_SOURCES ${OUTPUT_SOURCE}) endforeach()

  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/a.pb.cc
    COMMAND
      protoc --cpp_out=${CMAKE_BINARY_DIR}/ -I ${CMAKE_CURRENT_SOURCE_DIR}/test
      ${CMAKE_CURRENT_SOURCE_DIR}/test/a.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/a.proto protoc-gen-kun
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/
    COMMENT "generate pb")

  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/b.kun.h
    COMMAND
      protoc --plugin=${CMAKE_BINARY_DIR}/protoc-gen-kun
      --kun_out=${CMAKE_BINARY_DIR}/ -I ${CMAKE_CURRENT_SOURCE_DIR}/test
      ${CMAKE_CURRENT_SOURCE_DIR}/test/b.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/b.proto
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/
    COMMENT "generate kun")

  link_libraries(
    protobuf::libprotobuf
    protobuf::libprotoc
    absl::absl_check
    absl::absl_log
    absl::algorithm
    absl::base
    absl::bind_front
    absl::bits
    absl::btree
    absl::cleanup
    absl::cord
    absl::core_headers
    absl::debugging
    absl::die_if_null
    absl::dynamic_annotations
    absl::flags
    absl::flat_hash_map
    absl::flat_hash_set
    absl::function_ref
    absl::hash
    absl::if_constexpr
    absl::layout
    absl::log_initialize
    absl::log_globals
    absl::log_severity
    absl::memory
    absl::node_hash_map
    absl::node_hash_set
    absl::optional
    absl::random_distributions
    absl::random_random
    absl::span
    absl::status
    absl::statusor
    absl::strings
    absl::synchronization
    absl::time
    absl::type_traits
    absl::utility
    absl::variant
    GTest::gtest_main)

  enable_testing()

  add_executable(test_serialize test/test_serialize.cpp
                                ${CMAKE_BINARY_DIR}/a.pb.cc)

endif(WITH_TEST)
