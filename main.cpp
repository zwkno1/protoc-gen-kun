#include <generator.h>

int main(int argc, char* argv[])
{
    Generator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}