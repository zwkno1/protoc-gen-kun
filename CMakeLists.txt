cmake_minimum_required(VERSION 3.6)

project(protoc-gen-kun)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_BUILD_TYPE Release)

find_package(Protobuf)
find_package(absl)

include_directories(./)
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
  add_executable(test_kun test/test_kun.cpp)
  add_executable(test_pb test/test_pb.cpp test/a.pb.cc)

  target_link_libraries(
    test_pb
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

endif(WITH_TEST)
