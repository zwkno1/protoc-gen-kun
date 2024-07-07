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
        p.Emit({ { "class", desc_->name() } },
               R"cc(
               class $class$;
               )cc");
    }

    void GenerateDefine(Printer& p)
    {
        p.Emit(
          {
            { "class", desc_->name() },
            {
              "fields",
              [&] {
                  for (auto& field : fields_) {
                      field.GenerateMembers(p);
                  }
              },
            },
            { "funcs", [&] { GenerateFunctions(p); } },
          },
          R"cc(
          class $class$ 
          {
          public:
              $funcs$

              $fields$
          };
          )cc");
    }

    void GenerateFunctions(Printer& p)
    {
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

private:
    void GenerateFileds(Printer& p, const Descriptor* desc) const {}

    const Descriptor* desc_;
    Options options_;
    std::vector<FieldGenerator> fields_;
};