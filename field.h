#pragma once

#include <memory>
#include <string>

#include <protobuf.h>

#include "kun.h"

std::string GetTypeName(const FieldDescriptor* fd)
{
    static const std::string typenames[] = {
        "ERROR",         // 0 is reserved for errors
        "::int32_t",     // CPPTYPE_INT32
        "::int64_t",     // CPPTYPE_INT64
        "::uint32_t",    // CPPTYPE_UINT32
        "::uint64_t",    // CPPTYPE_UINT64
        "double",        // CPPTYPE_DOUBLE
        "float",         // CPPTYPE_FLOAT
        "bool",          // CPPTYPE_BOOL
        "enum",          // CPPTYPE_ENUM
        "::std::string", // CPPTYPE_STRING
        "class",         // CPPTYPE_MESSAGE
    };
    if (fd->cpp_type() == CppType::CPPTYPE_MESSAGE) {
        return google::protobuf::compiler::cpp::QualifiedClassName(fd->message_type());
    } else if (fd->cpp_type() == CppType::CPPTYPE_ENUM) {
        return fd->enum_type()->name();
    }
    return typenames[fd->cpp_type()];
}

uint64_t MakeTag(const FieldDescriptor* desc)
{
    kun::WireType t = kun::WIRE_VARINT;
    if (desc->is_repeated() || desc->is_map() || (desc->cpp_type() == CppType::CPPTYPE_MESSAGE)) {
        t = kun::WireType::WIRE_LENGTH_DELIM;
    } else {
        switch (desc->cpp_type()) {
        case CppType::CPPTYPE_INT32:
        case CppType::CPPTYPE_INT64:
        case CppType::CPPTYPE_UINT32:
        case CppType::CPPTYPE_UINT64:
        case CppType::CPPTYPE_ENUM:
        case CppType::CPPTYPE_BOOL:
            t = kun::WireType::WIRE_VARINT;
            break;
        case CppType::CPPTYPE_DOUBLE:
            t = kun::WireType::WIRE_FIXED64;
            break;
        case CppType::CPPTYPE_FLOAT:
            t = kun::WireType::WIRE_FIXED32;
            break;
        case CppType::CPPTYPE_STRING:
        case CppType::CPPTYPE_MESSAGE:
            t = kun::WireType::WIRE_LENGTH_DELIM;
            break;
        }
    }
    uint64_t tag = desc->number();
    return (tag << 3) | t;
}

class FieldGeneratorBase
{
public:
    FieldGeneratorBase(const FieldDescriptor* field, const Options& options)
      : field_(field)
      , options_(options)
    {
    }

    const FieldDescriptor* Field() const { return field_; }

    virtual void GenerateTemplate(Printer& p, const std::string& tp) const { p.Emit(tp); }

    virtual void GenerateMembers(Printer& p) const { p.Emit("$type$ $name$;\n"); }

    virtual void GenerateEncode(Printer& p) const
    {
        p.Emit(R"cc(
        if (::$kun_ns$::HasValue($name$)) {
            enc.Encode(__meta__[$meta_index$], $name$);
        }
        )cc");
    }

    virtual void GenerateDecode(Printer& p) const
    {
        p.Emit(R"cc(
        case __meta__[$meta_index$].tag: {
            return dec.Decode($name$);
        }
        )cc");
    }

    virtual void GenerateByteSize(Printer& p) const
    {
        p.Emit(R"cc(
        if (::$kun_ns$::HasValue($name$)) {
            total_size += ::$kun_ns$::ByteSizeWithTag<$tag$>($name$);
        } 
        )cc");
    }

    virtual void GenerateConstructor(Printer& p) const { p.Emit("$name$()\n"); }

    virtual void GenerateMeta(Printer& p) const
    {
        p.Emit(R"cc(::$kun_ns$::FieldMeta{ $number$, $tag$, "$name$" }, )cc");
    }

    std::vector<Printer::Sub> MakeVars() const
    {
        return {
            { "name", google::protobuf::compiler::cpp::FieldName(field_) },
            { "type", GetTypeName(field_) },
            { "number", field_->number() },
            { "index", field_->index() },
            { "tag", MakeTag(field_) },
        };
    }

    virtual ~FieldGeneratorBase(){};

protected:
    const FieldDescriptor* field_;
    const Options& options_;
};

class IntergerFieldGenerator : public FieldGeneratorBase
{
public:
    IntergerFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateConstructor(Printer& p) const override { p.Emit("$name$(0)\n"); }
};

class BooleanFieldGenerator : public FieldGeneratorBase
{
public:
    BooleanFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateConstructor(Printer& p) const override { p.Emit("$name$(false)\n"); }
};

class FixedFieldGenerator : public FieldGeneratorBase
{
public:
    FixedFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateConstructor(Printer& p) const override { p.Emit("$name$(0)\n"); }
};

class StringFieldGenerator : public FieldGeneratorBase
{
public:
    StringFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
};

class MessageFieldGenerator : public FieldGeneratorBase
{
public:
    MessageFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::optional<$type$> $name$;\n"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(
        if($name$) {
            total_size += ::$kun_ns$::ByteSizeWithTag<$tag$>(*$name$);
        }
        )cc");
    }

    void GenerateEncode(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if($name$) {
              enc.Encode(__meta__[$meta_index$], *$name$);
          }
          )cc");
    }

    void GenerateDecode(Printer& p) const override
    {
        p.Emit(R"cc(
        case __meta__[$meta_index$].tag: {
            $name$.emplace();
            return dec.Decode(*$name$);
        }
        )cc");
    }
};

class EnumFieldGenerator : public FieldGeneratorBase
{
public:
    EnumFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
};

class RepeatedIntergerFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedIntergerFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }
};

class RepeatedBooleanFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedBooleanFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }
};

class RepeatedFixedFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedFixedFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }
};

class RepeatedStringFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedStringFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }

    void GenerateDecode(Printer& p) const override
    {
        p.Emit(R"cc(
        case __meta__[$meta_index$].tag: {
            $type$ e;
            if (!dec.Decode(e)) {
                return false;
            }
            $name$.push_back(std::move(e));
            return true;
        }
        )cc");
    }
};

class RepeatedMessageFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedMessageFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }

    void GenerateDecode(Printer& p) const override
    {
        p.Emit(R"cc(
        case __meta__[$meta_index$].tag: {
            $type$ e;
            if (!dec.Decode(e)) {
                return false;
            }
            $name$.push_back(std::move(e));
            return true;
        }
        )cc");
    }
};

class RepeatedEnumFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedEnumFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }
};

class MapFieldGenerator : public FieldGeneratorBase
{
public:
    MapFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
    void GenerateMembers(Printer& p) const override
    {
        p.Emit(
          {
            { "key", GetTypeName(field_->message_type()->map_key()) },
            { "value", GetTypeName(field_->message_type()->map_value()) },
          },
          R"cc(
              std::unordered_map<$key$, $value$> $name$;
          )cc");
    }
};

class OneofFieldGenerator : public FieldGeneratorBase
{
public:
    OneofFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
    void GenerateMembers(Printer& p) const override {}

    void GenerateByteSize(Printer& p) const override {}
};

std::unique_ptr<FieldGeneratorBase> MakeGenerator(const FieldDescriptor* field, const Options& options)
{

    if (field->is_map()) {
        return std::make_unique<MapFieldGenerator>(field, options);
    }

    if (field->is_repeated()) {
        if (field->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
            return std::make_unique<RepeatedBooleanFieldGenerator>(field, options);
        } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
            return std::make_unique<RepeatedStringFieldGenerator>(field, options);
        } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
            return std::make_unique<RepeatedMessageFieldGenerator>(field, options);
        } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
            return std::make_unique<RepeatedEnumFieldGenerator>(field, options);
        } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE ||
                   field->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
            return std::make_unique<RepeatedFixedFieldGenerator>(field, options);
        }
        return std::make_unique<RepeatedIntergerFieldGenerator>(field, options);
    }

    if (field->real_containing_oneof() && field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        return std::make_unique<OneofFieldGenerator>(field, options);
    }

    if (field->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
        return std::make_unique<BooleanFieldGenerator>(field, options);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
        return std::make_unique<StringFieldGenerator>(field, options);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        return std::make_unique<MessageFieldGenerator>(field, options);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
        return std::make_unique<EnumFieldGenerator>(field, options);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE ||
               field->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
        return std::make_unique<FixedFieldGenerator>(field, options);
    }
    return std::make_unique<IntergerFieldGenerator>(field, options);
}

class FieldGenerator
{
public:
    FieldGenerator(const FieldDescriptor* field, const Options& options)
      : impl_(MakeGenerator(field, options))
    {
    }

    void GenerateMembers(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateMembers(p);
    }

    void GenerateEncode(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateEncode(p);
    }

    void GenerateDecode(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateDecode(p);
    }

    void GenerateConstructor(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateConstructor(p);
    }

    void GenerateByteSize(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateByteSize(p);
    }

    void GenerateMeta(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateMeta(p);
    }

    void GenerateTemplate(Printer& p, const std::string& tp) const
    {
        auto v = p.WithVars(MakeVars());
        impl_->GenerateTemplate(p, tp);
    }

    std::vector<Printer::Sub> MakeVars() const { return impl_->MakeVars(); }

    std::unique_ptr<FieldGeneratorBase> impl_;
};