#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <enum.h>
#include <message.h>

#include "header.h"
#include "protobuf.h"

class Generator : public google::protobuf::compiler::CodeGenerator
{
public:
    bool Generate(const FileDescriptor* file, const std::string& parameter, GeneratorContext* generator_context,
                  std::string* error) const override
    {

        Options options = ParseOptions(parameter);

        std::string basename = google::protobuf::compiler::StripProto(file->name());

        std::unique_ptr<ZeroCopyOutputStream> output(generator_context->Open(basename + ".kun.h"));

        Printer p(output.get(), Printer::Options{});

        auto ns = google::protobuf::compiler::cpp::Namespace(file);
        auto varCleaner = p.WithVars({
          { "kun_ns", "kun" },
          { "ns", ns },
        });

        GenerateHeader(file, p);

        std::vector<const EnumDescriptor*> enumDescs;
        std::vector<const Descriptor*> messageDescs;
        GetAllDescriptor(file, messageDescs, enumDescs);

        std::vector<EnumGenerator> enums;
        std::vector<MessageGenerator> messages;

        for (auto desc : enumDescs) {
            enums.push_back(EnumGenerator(desc, options));
        }

        for (auto desc : messageDescs) {
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

    void GetAllDescriptor(const FileDescriptor* file, std::vector<const Descriptor*>& messages,
                          std::vector<const EnumDescriptor*>& enums) const
    {

        std::function<void(const Descriptor*)> travel;
        travel = [&](const Descriptor* desc) {
            if (google::protobuf::compiler::cpp::IsMapEntryMessage(desc)) {
                return;
            }

            // std::cout << "desc: " << desc->name() << std::endl;

            messages.push_back(desc);
            for (int i = 0; i < desc->enum_type_count(); i++) {
                // std::cout << "desc: " << desc->enum_type(i)->name() << std::endl;
                enums.push_back(desc->enum_type(i));
            }

            for (int i = 0; i < desc->nested_type_count(); i++) {
                auto d = desc->nested_type(i);
                travel(d);
            }
        };

        for (int i = 0; i < file->enum_type_count(); i++) {
            enums.push_back(file->enum_type(i));
        }

        for (int i = 0; i < file->message_type_count(); i++) {
            auto desc = file->message_type(i);
            travel(desc);
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