#pragma once
#include <protobuf.h>

class EnumGenerator
{
public:
    EnumGenerator(const EnumDescriptor* desc, const Options& options)
      : desc_(desc)
      , options_(options)
    {
    }

    void GenerateDefine(Printer& p)
    {
        p.Emit(
          {
            { "enum", desc_->name() },
            { "body",
              [&] {
                  for (int j = 0; j < desc_->value_count(); j++) {
                      auto v = desc_->value(j);
                      p.Emit(
                        {
                          { "name", v->name() },
                          { "value", std::to_string(v->number()) },
                        },
                        "$name$ = $value$,\n");
                  }
              } },
          },
          R"cc(
          enum class $enum$ : int32_t
          {
              $body$
          };
          )cc");
    }

    void GenerateHelperFunctions(Printer& p) const
    {
        p.Emit(
          {
            { "enum", desc_->name() },
          },
          R"cc(
          template<>
          struct is_enum<$ns$::$enum$> : public std::true_type
          {
          };
          )cc");
    }

private:
    const EnumDescriptor* desc_;
    Options options_;
};