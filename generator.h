#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <enum.h>
#include <message.h>

#include "header.h"

class Generator : public google::protobuf::compiler::CodeGenerator
{
public:
    bool Generate(const FileDescriptor* file, const std::string& parameter, GeneratorContext* generator_context,
                  std::string* error) const override
    {

        Options options = ParseOptions(parameter);

        std::string basename = google::protobuf::compiler::StripProto(file->name());

        std::unique_ptr<ZeroCopyOutputStream> output(generator_context->Open(basename + ".kun.h"));

        file->package();

        Printer p(output.get(), Printer::Options{});

        auto ns = google::protobuf::compiler::cpp::Namespace(file);
        auto varCleaner = p.WithVars({
          { "kun_ns", "kun" },
          { "ns", ns },
        });

        GenerateHeader(p);

        std::vector<EnumGenerator> enums;
        std::vector<MessageGenerator> messages;

        for (int i = 0; i < file->enum_type_count(); i++) {
            auto desc = file->enum_type(i);
            enums.push_back(EnumGenerator(desc, options));
        }

        for (int i = 0; i < file->message_type_count(); i++) {
            auto desc = file->message_type(i);
            messages.push_back(MessageGenerator(desc, options));
        }

        {
            NamespaceOpener nso(ns, &p);

            p.Print("\n");
            for (auto& i : messages) {
                i.GenerateForwardDeclare(p);
                p.Print("\n");
            }

            for (auto& i : enums) {
                i.GenerateDefine(p);
                p.Print("\n");
            }

            for (auto& i : messages) {
                i.GenerateDefine(p);
                p.Print("\n");
            }
        }

        {
            NamespaceOpener nso("kun", &p);

            for (auto& i : enums) {
                i.GenerateHelperFunctions(p);
                p.Print("\n");
            }

            for (auto& i : messages) {
                i.GenerateHelperFunctions(p);
                p.Print("\n");
            }
        }

        return true;
    }

    void GetAllMessage(const FileDescriptor* file) const
    {
        std::vector<const Descriptor*> messages;
        std::unordered_map<std::string, const Descriptor*> n;
        for (int i = 0; i < file->message_type_count(); i++) {
            auto m = file->message_type(i);
            messages.push_back(file->message_type(i));
        }

        for (auto m : messages) {
            GetNestedMessage(m);
        }
    }

    void GetNestedMessage(const Descriptor* m) const
    {
        if (google::protobuf::compiler::cpp::IsMapEntryMessage(m)) {
            return;
        }
        // std::cout << "msg: " << m->full_name() << "," << m->is_placeholder() << ", " << m.<< std::endl;
        for (int i = 0; i < m->nested_type_count(); i++) {
            auto nm = m->nested_type(i);
            GetNestedMessage(nm);
        }
    }

    Options ParseOptions(const std::string& parameter) const
    {
        std::vector<std::pair<std::string, std::string>> file_options;
        google::protobuf::compiler::ParseGeneratorParameter(parameter, &file_options);
        Options options;
        // TODO: parse options
        return options;
    }
};