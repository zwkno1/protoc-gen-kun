#pragma once
#include <cstddef>
#include <memory>
#include <ranges>
#include <vector>

#include <field.h>
#include <google/protobuf/descriptor.h>
#include <protobuf.h>

class MessageGenerator
{
public:
    MessageGenerator(const Descriptor* desc, const Options& options)
      : desc_(desc)
      , options_(options)
    {
        std::vector<const FieldDescriptor*> fields;
        for (int i = 0; i < desc_->field_count(); i++) {
            fields.push_back(desc->field(i));
        }

        // std::ranges::sort(fields, [&](auto x, auto y) { return x->number() < y->number(); });

        for (size_t i = 0; i < fields.size(); i++) {
            auto field = fields[i];
            fields_.push_back(FieldGenerator(field, i, options));
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

              mutable size_t _cached_size_;
          };
          )cc");
    }

    void GenerateFunctions(Printer& p)
    {
        auto clean = p.WithVars(MakeVars());
        p.Emit(
          {
            {
              "encode_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      fields_[i].GenerateEncode(p);
                      if (i != fields_.size() - 1) {
                          p.Print("\n");
                      }
                  }
              },
            },
            {
              "decode_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      fields_[i].GenerateDecode(p);
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
              "copy_constructor_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      if (i != 0) {
                          p.Print(", ");
                      }
                      auto& field = fields_[i];
                      field.GenerateCopyConstructor(p);
                  }
              },
            },
            {
              "move_constructor_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      if (i != 0) {
                          p.Print(", ");
                      }
                      auto& field = fields_[i];
                      field.GenerateMoveConstructor(p);
                  }
              },
            },
            {
              "assignment_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateAssignment(p);
                  }
              },
            },
            {
              "move_assignment_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateMoveAssignment(p);
                  }
              },
            },
            {
              "equal_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateEqual(p);
                  }
              },
            },
            {
              "bytesize_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateByteSize(p);
                      p.Print("\n");
                  }
              },
            },
          },
          R"cc(
            $class$()
              : $constructor_body$
              , _cached_size_(0)
            {
            }

            $class$(const $class$& other)
              : $copy_constructor_body$
            {
            }

            $class$($class$&& other)
              : $move_constructor_body$
            {
            }

            $class$& operator=(const $class$& other)
            {
                $assignment_body$
                return *this;
            }

            $class$& operator=($class$&& other)
            {
                $move_assignment_body$
                return *this;
            }

            bool operator==(const $class$& other) const
            {
                $equal_body$
                return true;
            }

            template <typename Encoder>
            inline void Encode(Encoder& enc) const
            {
                $encode_body$
            }

            template <typename Decoder>
            inline bool Decode(Decoder& dec, uint64_t tag) 
            {
                switch (tag) {
                $decode_body$
                }

                return true;
            }

            inline size_t ByteSize() const
            {
                size_t total_size = 0;
                $bytesize_body$
                _cached_size_ = total_size;
                return total_size;
            }
            )cc");
    }

    void GenerateHelperFunctions(Printer& p) const
    {
        auto v = p.WithVars(MakeVars());
        p.Emit(
          R"cc(
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
            { "qualified_class", google::protobuf::compiler::cpp::QualifiedClassName(desc_) },
        };
    }

private:
    const Descriptor* desc_;
    Options options_;
    std::vector<FieldGenerator> fields_;
};