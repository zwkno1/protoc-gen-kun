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
                      p.Emit({ { "name", v->name() }, { "value", std::to_string(v->index()) } }, "$name$ = $value$,\n");
                  }
              } },
          },
          R"cc(
          enum $enum$ : int
          {
              $body$
          };
          )cc");
    }

private:
    const EnumDescriptor* desc_;
    Options options_;
};