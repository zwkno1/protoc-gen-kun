#pragma once
#include <cstddef>
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
            sortedIndex_.push_back(i);
        }

        std::ranges::sort(sortedIndex_,
                          [&](size_t x, size_t y) { return desc_->field(x)->number() < desc_->field(y)->number(); });
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
                      auto clean = p.WithVars({ { "meta_index", i } });
                      fields_[i].GenerateEncode(p);
                      if (i != sortedIndex_.size() - 1) {
                          p.Print("\n");
                      }
                  }
              },
            },
            {
              "decode_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto clean = p.WithVars({ { "meta_index", i } });
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
                      field.GenerateTemplate(p, "$name$(other.$name$)\n");
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
                      field.GenerateTemplate(p, "$name$(std::move(other.$name$))\n");
                  }
              },
            },
            {
              "assignment_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateTemplate(p, "$name$ = other.$name$;\n");
                  }
              },
            },
            {
              "move_assignment_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateTemplate(p, "$name$ = std::move(other.$name$);\n");
                  }
              },
            },
            {
              "equal_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      field.GenerateTemplate(p, R"cc(
                          if ($name$ != other.$name$) {
                              return false;
                          }
                          )cc");
                  }
              },
            },
            {
              "bytesize_body",
              [&] {
                  for (size_t i = 0; i < fields_.size(); i++) {
                      auto& field = fields_[i];
                      auto clean = p.WithVars({ { "meta_index", i } });
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
          //{
          //  { "field_num", fields_.size() },
          //  { "field_meta",
          //    [&] {
          //        for (auto i : sortedIndex_) {
          //            fields_[i].GenerateMeta(p);
          //            p.Print("\n");
          //        }
          //    } },
          //},
          R"cc(
            template<>
            struct is_message<$ns$::$class$> : public std::true_type
            {
            };
            )cc");

        // template<>
        // struct message_meta<$ns$::$class$>
        //{
        //     inline constexpr static std::array<::$kun_ns$::FieldMeta, $field_num$> __meta__ = {
        //         $field_meta$
        //     };
        // };
        //)cc");
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
    std::vector<size_t> sortedIndex_;
};