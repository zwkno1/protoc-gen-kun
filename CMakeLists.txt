cmake_minimum_required(VERSION 3.6)

project(protoc-gen-kun)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
# set(CMAKE_BUILD_TYPE Release)
option(WITH_TESTS "enable test" ON)

find_package(Protobuf)
find_package(absl)

include_directories(./ ${GTEST_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}
                    ${Protobuf_INCLUDE_DIRS})
add_executable(protoc-gen-kun main.cpp)
target_link_libraries(protoc-gen-kun protobuf::libprotobuf protobuf::libprotoc
                      absl::absl_check absl::flat_hash_set absl::strings)

if(WITH_TEST)
  find_package(GTest)
  # set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/a.proto") file(GLOB PROTO_FILES
  # "${PROTO_PATH}/*.proto") foreach(PROTO_FILE in ${PROTO_FILES}) string(REGEX
  # REPLACE "[.]proto$" ".pb.cc" OUTPUT_SOURCE ${PROTO_FILE}) list(APPEND
  # OUTPUT_SOURCES ${OUTPUT_SOURCE}) endforeach()
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/a.kun.h
    COMMAND
      protoc --plugin=${CMAKE_BINARY_DIR}/protoc-gen-kun
      --kun_out=${CMAKE_BINARY_DIR}/ -I ${CMAKE_CURRENT_SOURCE_DIR}/test
      ${CMAKE_CURRENT_SOURCE_DIR}/test/a.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/a.proto
            ${CMAKE_BINARY_DIR}/protoc-gen-kun
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/
    COMMENT "generate kun")

  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/b.pb.cc
    COMMAND
      protoc --cpp_out=${CMAKE_BINARY_DIR}/ -I ${CMAKE_CURRENT_SOURCE_DIR}/test
      ${CMAKE_CURRENT_SOURCE_DIR}/test/b.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/b.proto
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/
    COMMENT "generate pb")

  link_libraries(protobuf::libprotobuf protobuf::libprotoc absl::absl_check
                 absl::flat_hash_set absl::status GTest::gtest_main)

  include(GoogleTest)
  enable_testing()

  add_executable(
    encode_pb_test test/encode_pb_test.cpp ${CMAKE_BINARY_DIR}/b.pb.cc
                   ${CMAKE_BINARY_DIR}/a.kun.h)

  add_executable(
    decode_pb_test test/decode_pb_test.cpp ${CMAKE_BINARY_DIR}/b.pb.cc
                   ${CMAKE_BINARY_DIR}/a.kun.h)

  gtest_discover_tests(encode_pb_test)
  gtest_discover_tests(decode_pb_test)

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate
  # -fcoverage-mapping -pg")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
  set(CMAKE_MODULE_PATH
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

  find_package(Gperftools REQUIRED)
  include_directories(GPERFTOOLS_INCLUDE_DIR)

  add_executable(bench test/benchmark.cpp ${CMAKE_BINARY_DIR}/b.pb.cc
                       ${CMAKE_BINARY_DIR}/a.kun.h)
  target_link_libraries(bench ${GPERFTOOLS_LIBRARIES})

endif(WITH_TEST)
