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
        return fd->message_type()->name();
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
          R"cc(b.WritePrimitive($number$, $name$);
          )cc");
    }

    virtual void GenerateByteSize(Printer& p) const
    { /*p.Emit("total_size += ::ByteSize($name$);\n");*/
    }

    virtual void GenerateConstructor(Printer& p) const { p.Emit("$name$()\n"); }

    std::vector<Printer::Sub> MakeVars()
    {
        return {
            { "name", google::protobuf::compiler::cpp::FieldName(field_) },
            { "type", GetTypeName(field_) },
            { "number", field_->number() },
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
              b.WriteInterger($number$, $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
        if ($name$ != 0) {
            total_size += ::TagSize($number$) + ::IntergerByteSize($name$);
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
          $tmp_type$ tmp_$name$;
          std::memcpy(&tmp_$name$, &$name$, sizeof($name$));
          if (tmp_$name$ != 0) {
              b.WriteFixed($number$, tmp_$name$);
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
          $tmp_type$ tmp_$name$;
          std::memcpy(&tmp_$name$, &$name$, sizeof($name$));
          if (tmp_$name$ != 0) {
              total_size += ::TagSize($number$) + sizeof($tmp_type$);
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
              b.WriteString($number$, $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              uint64_t size = $name$.size();
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
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
              b.WriteMessage($number$, *$name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if($name$) {
              uint64_t size = $name$->ByteSize();
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
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
              b.WriteEnum($number$, $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if ($name$ != 0) {
              total_size += ::TagSize($number$) + ::IntergerByteSize($name$);
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              b.WriteIntergerPacked($number$, $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              uint64_t size = ::RepeatIntergerByteSize($name$);
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
          }
          )cc");
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              b.WriteFixedPacked($number$, $name$);
          }
          )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              uint64_t size = ::RepeatedFixedByteSize($name$);
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
          }
          )cc");
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          for (auto& entry : $name$) {
              b.WriteString($number$, entry);
          }
       )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          for (auto& entry : $name$) {
              uint64_t size = entry.size();
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          for (auto& entry : $name$) {
              b.WriteMessage($number$, entry);
          }
       )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          for (auto& entry : $name$) {
              uint64_t size = entry.ByteSize();
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          for (auto& entry : $name$) {
              b.WriteEnum($number$, entry);
          }
       )cc");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if (!$name$.empty()) {
              uint64_t size = ::RepeatIntergerByteSize($name$);
              total_size += ::TagSize($number$) + ::LengthDelimitedSize(size);
          }
          )cc");
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

    void GenerateSerialize(Printer& p) const override
    {
        p.Emit(R"cc(
        if (!$name$.empty()) {
            b.WriteMap($number$, $name$);
        }
        )cc");
    }

    void GenerateByteSize(Printer& p) const override {}
};

class OneofFieldGenerator : public FieldGeneratorBase
{
public:
    OneofFieldGenerator(const FieldDescriptor* field, const Options& options)
      : FieldGeneratorBase(field, options)
    {
    }
    void GenerateMembers(Printer& p) const override {}
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

    std::unique_ptr<FieldGeneratorBase> impl_;
};