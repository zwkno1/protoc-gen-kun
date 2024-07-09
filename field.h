#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <protobuf.h>

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
        "class2",        // CPPTYPE_MESSAGE
    };
    if (fd->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
        return google::protobuf::compiler::cpp::ClassName(fd->message_type());
    } else if (fd->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_ENUM) {
        return fd->enum_type()->name();
    }
    return typenames[fd->cpp_type()];
}

class FieldGeneratorBase
{
public:
    FieldGeneratorBase(const FieldDescriptor* field, const Options& options)
      : field_(field)
      , options_(options)
    {
    }

    virtual void GenerateMembers(Printer& p) const { p.Emit("$type$ $name$;\n"); }

    virtual void GenerateSerialize(Printer& p) const
    {
        p.Emit(
          R"cc(b.Write(__meta__[$index$], $name$);
          )cc");
    }

    virtual void GenerateByteSize(Printer& p) const = 0;

    virtual void GenerateConstructor(Printer& p) const { p.Emit("$name$()\n"); }

    virtual void GenerateMeta(Printer& p) const { p.Emit(R"cc(::$kun_ns$::FieldMeta{ $number$, "$name$" }, )cc"); }

    std::vector<Printer::Sub> MakeVars()
    {
        return {
            { "name", google::protobuf::compiler::cpp::FieldName(field_) },
            { "type", GetTypeName(field_) },
            { "number", field_->number() },
            { "index", field_->index() },
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if ($name$ != 0) {
              b.Write(__meta__[$index$], $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
        if ($name$ != 0) {
            total_size += ::$kun_ns$::ByteSize<$number$>($name$);
        }
        )cc");
    }
};

class FixedFieldGenerator : public FieldGeneratorBase
{
public:
    FixedFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateConstructor(Printer& p) const override { p.Emit("$name$(0)\n"); }

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          {
            {
              "tmp_type",
              [&] { return field_->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE ? "::uint64_t" : "::uint32_t"; }(),
            },
          },
          R"cc(
          $tmp_type$ tmp_$name$ = std::bit_cast<$tmp_type$>($name$);
          if (tmp_$name$ != 0) {
              b.Write(__meta__[$index$], $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          {
            {
              "tmp_type",
              [&] { return field_->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE ? "::uint64_t" : "::uint32_t"; }(),
            },
          },
          R"cc(
          $tmp_type$ tmp_$name$ = std::bit_cast<$tmp_type$>($name$);
          if (tmp_$name$ != 0) {
              total_size += ::$kun_ns$::ByteSize<$number$>($name$);
          }
          )cc");
    }
};

class StringFieldGenerator : public FieldGeneratorBase
{
public:
    StringFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              b.Write(__meta__[$index$], $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              total_size += ::$kun_ns$::ByteSize<$number$>($name$);
          }
          )cc");
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if($name$) {
              b.Write(__meta__[$index$], *$name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if($name$) {
              total_size += ::$kun_ns$::ByteSize<$number$>(*$name$);
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if ($name$ != 0) {
              b.Write(__meta__[$index$], static_cast<uint64_t>($name$));
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if ($name$ != 0) {
              total_size += ::$kun_ns$::ByteSize<$number$>($name$);
          }
          )cc");
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

    void GenerateSerialize(Printer& p) const override { p.Emit(R"cc(b.Write(__meta__[$index$], $name$);)cc"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(total_size += ::$kun_ns$::ByteSize<$number$>($name$);)cc");
    }
};

class RepeatedFixedFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedFixedFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }

    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }

    void GenerateSerialize(Printer& p) const override { p.Emit(R"cc(b.Write(__meta__[$index$], $name$);)cc"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(total_size += ::$kun_ns$::ByteSize<$number$>($name$);)cc");
    }
};

class RepeatedStringFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedStringFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
    void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }

    void GenerateSerialize(Printer& p) const override { p.Emit(R"cc(b.Write(__meta__[$index$], $name$);)cc"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(total_size += ::$kun_ns$::ByteSize<$number$>($name$);)cc");
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

    void GenerateSerialize(Printer& p) const override { p.Emit(R"cc(b.Write(__meta__[$index$], $name$);)cc"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(total_size += ::$kun_ns$::ByteSize<$number$>($name$);)cc");
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

    void GenerateSerialize(Printer& p) const override { p.Emit(R"cc(b.Write(__meta__[$index$], $name$);)cc"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(total_size += ::$kun_ns$::ByteSize<$number$>($name$);)cc");
    }
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

    void GenerateSerialize(Printer& p) const override { p.Emit(R"cc(b.Write(__meta__[$index$], $name$);)cc"); }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(total_size += ::$kun_ns$::ByteSize<$number$>($name$);)cc");
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
        if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
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

    if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
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
        auto v = p.WithVars(impl_->MakeVars());
        impl_->GenerateMembers(p);
    }

    void GenerateSerialize(Printer& p) const
    {
        auto v = p.WithVars(impl_->MakeVars());
        impl_->GenerateSerialize(p);
    }

    void GenerateConstructor(Printer& p) const
    {
        auto v = p.WithVars(impl_->MakeVars());
        impl_->GenerateConstructor(p);
    }

    void GenerateByteSize(Printer& p) const
    {
        auto v = p.WithVars(impl_->MakeVars());
        impl_->GenerateByteSize(p);
    }

    void GenerateMeta(Printer& p) const
    {
        auto v = p.WithVars(impl_->MakeVars());
        impl_->GenerateMeta(p);
    }

    std::unique_ptr<FieldGeneratorBase> impl_;
};