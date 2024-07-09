#pragma once
#include <memory>
#include <ranges>
#include <vector>

#include <field.h>
#include <protobuf.h>

class MessageGenerator
{
public:
    MessageGenerator(const Descriptor* desc, const Options& options)
      : desc_(desc)
      , options_(options)
    {
        for (int i = 0; i < desc_->field_count(); i++) {
            auto field = desc_->field(i);
            fields_.push_back(FieldGenerator(field, options));
        }
    }

    void GenerateForwardDeclare(Printer& p)
    {
        auto v = p.WithVars(MakeVars());
        p.Emit(R"cc(class $class$;
               )cc");
    }

    void GenerateDefine(Printer& p)
    {
        auto v = p.WithVars(MakeVars());
        p.Emit(
          {
            {
              "fields",
              [&] {
                  for (auto& field : fields_) {
                      field.GenerateMembers(p);
                  }
              },
            },
            { "funcs", [&] { GenerateFunctions(p); } },
            { "meta", [&] { GenerateMeta(p); } },
          },
          R"cc(
          class $class$ 
          {
          public:
              $meta$

              $funcs$

              $fields$
          };
          )cc");
    }

    void GenerateFunctions(Printer& p)
    {
        auto v = p.WithVars(MakeVars());
        p.Emit(
          {
            {
              "serialize_body",
              [&] {
                  std::vector<const FieldDescriptor*> fields;
                  for (int i = 0; i < desc_->field_count(); i++) {
                      fields.push_back(desc_->field(i));
                  }

                  std::ranges::sort(fields, [](const FieldDescriptor* x, const FieldDescriptor* y) {
                      return x->number() < y->number();
                  });

                  for (auto& field : fields) {
                      fields_[field->index()].GenerateSerialize(p);
                      p.Print("\n");
                  }
              },
            },
            {
              "constructor_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      if (i != 0) {
                          p.Print(", ");
                      }
                      auto& field = fields_[i];
                      field.GenerateConstructor(p);
                  }
              },
            },
            {
              "bytesize_body",
              [&] {
                  for (auto& field : fields_) {
                      field.GenerateByteSize(p);
                      p.Print("\n");
                  }
              },
            },
          },
          R"cc(
            $class$()
              : $constructor_body$
            {
            }

            template <typename Buffer>
            inline void Serialize(Buffer& b) const
            {
                $serialize_body$
            }

            inline size_t ByteSize() const
            {
                size_t total_size = 0;
                $bytesize_body$
                return total_size;
            }
            )cc");
    }

    void GenerateHelperFunctions(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        p.Emit(R"cc(
            template<>
            struct is_message<$ns$::$class$> : public std::true_type
            {
            };
          )cc");
    }

    void GenerateMeta(Printer& p) const
    {
        p.Emit(
          {
            { "field_num", fields_.size() },
            { "field_meta",
              [&] {
                  for (auto& field : fields_) {
                      field.GenerateMeta(p);
                      p.Print("\n");
                  }
              } },
          },
          R"cc(
          inline constexpr static std::array<::$kun_ns$::FieldMeta, $field_num$> __meta__ = {
              $field_meta$
          };)cc");
    }

    std::vector<Printer::Sub> MakeVars() const
    {
        return {
            { "class", google::protobuf::compiler::cpp::ClassName(desc_) },
        };
    }

private:
    const Descriptor* desc_;
    Options options_;
    std::vector<FieldGenerator> fields_;
};