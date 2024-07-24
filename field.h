#pragma once

#include <memory>
#include <string>
#include <string_view>

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

kun::EncodingType GetEncodingType(const FieldDescriptor* field)
{
    switch (field->type()) {
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_DOUBLE:
    case FieldDescriptor::TYPE_FLOAT:
        return kun::ENCODING_FIXED;
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SINT64:
        return kun::ENCODING_ZIGZAG;
    case FieldDescriptor::TYPE_INT32:
    case FieldDescriptor::TYPE_INT64:
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_UINT64:
    case FieldDescriptor::TYPE_ENUM:
    case FieldDescriptor::TYPE_BOOL:
        return kun::ENCODING_VARINT;
    case FieldDescriptor::TYPE_STRING:
        return kun::ENCODING_UTF8;
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_GROUP:
    case FieldDescriptor::TYPE_MESSAGE:
    }
    return kun::ENCODING_NONE;
}

uint64_t MakeTag(const FieldDescriptor* desc)
{
    kun::WireType t = kun::WIRE_VARINT;
    if (desc->is_repeated() || desc->is_map() || (desc->cpp_type() == CppType::CPPTYPE_MESSAGE)) {
        t = kun::WireType::WIRE_LENGTH_DELIM;
    } else {
        switch (desc->type()) {
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_UINT32:
        case FieldDescriptor::TYPE_UINT64:
        case FieldDescriptor::TYPE_ENUM:
        case FieldDescriptor::TYPE_BOOL:
            t = kun::WireType::WIRE_VARINT;
            break;
        case FieldDescriptor::TYPE_FIXED32:
        case FieldDescriptor::TYPE_SFIXED32:
        case FieldDescriptor::TYPE_FLOAT:
            t = kun::WireType::WIRE_FIXED32;
            break;
        case FieldDescriptor::TYPE_FIXED64:
        case FieldDescriptor::TYPE_SFIXED64:
        case FieldDescriptor::TYPE_DOUBLE:
            t = kun::WireType::WIRE_FIXED64;
            break;
        case FieldDescriptor::TYPE_MESSAGE:
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES:
        case FieldDescriptor::TYPE_GROUP:
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
            enc.template Encode<$class$, $meta_index$>($name$);
        }
        )cc");
    }

    virtual void GenerateDecode(Printer& p) const
    {
        p.Emit(R"cc(
        case __meta__[$meta_index$].tag: {
            return dec.template Decode<$class$, $meta_index$>($name$);
        }
        )cc");
    }

    virtual void GenerateByteSize(Printer& p) const
    {
        p.Emit(R"cc(
        if (::$kun_ns$::HasValue($name$)) {
            total_size += ::$kun_ns$::ByteSizeWithTag<$class$, $meta_index$>($name$);
        } 
        )cc");
    }

    virtual void GenerateConstructor(Printer& p) const { p.Emit("$name$()\n"); }

    virtual void GenerateMeta(Printer& p) const
    {
        p.Emit(R"cc(::$kun_ns$::FieldMeta{ $number$, $tag$, $encoding$, "$name$" }, )cc");
    }

    std::vector<Printer::Sub> MakeVars() const
    {
        uint32_t c = kun::ENCODING_NONE;

        if (field_->is_map()) {
            c = (GetEncodingType(field_->message_type()->map_key()) << 8) |
              GetEncodingType(field_->message_type()->map_value());
        } else {
            c = GetEncodingType(field_);
        }

        return {
            { "name", google::protobuf::compiler::cpp::FieldName(field_) },
            { "type", GetTypeName(field_) },
            { "number", field_->number() },
            { "index", field_->index() },
            { "tag", MakeTag(field_) },
            { "encoding", c },
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

class FloatingFieldGenerator : public FieldGeneratorBase
{
public:
    FloatingFieldGenerator(const FieldDescriptor* field, const Options& options)
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
            total_size += ::$kun_ns$::ByteSizeWithTag<$class$, $meta_index$>(*$name$);
        }
        )cc");
    }

    void GenerateEncode(Printer& p) const override
    {
        p.Emit(
          R"cc(
          if($name$) {
              enc.template Encode<$class$, $meta_index$>(*$name$);
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
      , encoding_(GetEncodingType(field))
    {
    }

    void GenerateMembers(Printer& p) const override
    {
        p.Emit("std::vector<$type$> $name$;\n");
        if (encoding_ != kun::ENCODING_FIXED) {
            p.Emit("mutable size_t _$name$_cached_size_;\n");
        }
    }

    void GenerateByteSize(Printer& p) const override
    {
        if (encoding_ != kun::ENCODING_FIXED) {
            p.Emit(R"cc(
            {
                size_t data_size = ::$kun_ns$::ByteSize<$encoding$>($name$);

                _$name$_cached_size_ = data_size;

                if(data_size != 0) {
                    total_size += ::$kun_ns$::TagSize($tag$) + ::$kun_ns$::LengthDelimitedSize(data_size);
                }
            }
            )cc");
        } else {
            p.Emit(R"cc(
            {
                size_t data_size = ::$kun_ns$::ByteSize<$encoding$>($name$);
                if(data_size != 0) {
                    total_size += ::$kun_ns$::TagSize($tag$) + ::$kun_ns$::LengthDelimitedSize(data_size);
                }
            }
            )cc");
        }
    }

    void GenerateEncode(Printer& p) const override
    {
        if (encoding_ != kun::ENCODING_FIXED) {
            p.Emit(R"cc(
            if (::$kun_ns$::HasValue($name$)) {
                enc.template Encode<$class$, $meta_index$>($name$, _$name$_cached_size_);
            }
            )cc");
        } else {
            p.Emit(R"cc(
            if (::$kun_ns$::HasValue($name$)) {
                enc.template Encode<$class$, $meta_index$>($name$);
            }
            )cc");
        }
    }

    // void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }
private:
    uint32_t encoding_;
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

class RepeatedFloatingFieldGenerator : public FieldGeneratorBase
{
public:
    RepeatedFloatingFieldGenerator(const FieldDescriptor* field, const Options& options)
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
            if (!dec.template Decode<$class$, $meta_index$>(e)) {
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

    void GenerateMembers(Printer& p) const override
    {
        p.Emit("std::vector<$type$> $name$;\n");
        p.Emit("mutable size_t _$name$_cached_size_;\n");
    }

    void GenerateByteSize(Printer& p) const override
    {
        p.Emit(R"cc(
            {
                size_t data_size = ::$kun_ns$::ByteSize<$encoding$>($name$);

                _$name$_cached_size_ = data_size;

                if(data_size != 0) {
                    total_size += ::$kun_ns$::TagSize($tag$) + ::$kun_ns$::LengthDelimitedSize(data_size);
                }
            }
            )cc");
    }

    void GenerateEncode(Printer& p) const override
    {
        p.Emit(R"cc(
            if (::$kun_ns$::HasValue($name$)) {
                enc.template Encode<$class$, $meta_index$>($name$, _$name$_cached_size_);
            }
            )cc");
    }

    // void GenerateMembers(Printer& p) const override { p.Emit("std::vector<$type$> $name$;\n"); }
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
            return std::make_unique<RepeatedFloatingFieldGenerator>(field, options);
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
        return std::make_unique<FloatingFieldGenerator>(field, options);
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