#pragma once

#include <memory>
#include <string>
#include <string_view>

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

        GenerateHeader(p);
        NamespaceOpener ns(google::protobuf::compiler::cpp::Namespace(file), &p);

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

        return true;
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