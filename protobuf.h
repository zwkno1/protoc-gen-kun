#pragma once

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/cpp/file.h>
#include <google/protobuf/compiler/cpp/generator.h>
#include <google/protobuf/compiler/cpp/names.h>
#include <google/protobuf/compiler/cpp/namespace_printer.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/json/json.h>

using google::protobuf::compiler::GeneratorContext;
using google::protobuf::io::ZeroCopyOutputStream;

using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::compiler::cpp::Options;
using google::protobuf::io::Printer;
using google::protobuf::compiler::cpp::NamespaceOpener;