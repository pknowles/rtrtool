// Copyright (c) 2024 Pyarelal Knowles, MIT License

#include <app.hpp>
#include <args.hxx>
#include <decodeless/pmr_writer.hpp>
#include <filesystem>
#include <rtrtool/converter.hpp>

namespace fs = std::filesystem;

// Bigger than anything practical. Only consume virtual address space.
#define MAX_FILE_SIZE (16llu << 30) // 16gb maximum

// TODO: avoid creating the output file if the input deos not exist? does not
// validate? conversion fails? What about files that don't fit in resident
// memory - waste of swap?
struct RTRConvertedFile {
    RTRConvertedFile(const fs::path& output, const fs::path& input)
        : m_file(output, MAX_FILE_SIZE) {
        rtrtool::convertFromGltf(m_file.allocator(), input);
    }
    const rtr::RootHeader& operator*() const {
        return *reinterpret_cast<const rtr::RootHeader*>(m_file.data());
    }
    decodeless::pmr_file_writer m_file;
};

struct RTRConvertedMemory {
    RTRConvertedMemory(const fs::path& input)
        : m_memory(MAX_FILE_SIZE) {
        rtrtool::convertFromGltf(m_memory.allocator(), input);
    }
    const rtr::RootHeader& operator*() const {
        return *reinterpret_cast<const rtr::RootHeader*>(m_memory.data());
    }
    decodeless::pmr_memory_writer m_memory;
};

int main(int argc, char* argv[]) {
    args::ArgumentParser parser("rtrtool: Ready to render (*.rtr) viewer and tool");
    args::Group required(parser, "Required positional arguments:", args::Group::Validators::All);
    args::Positional<std::string> output(parser, "output",
                                         "Output rtr file to write. Will view input if not given.");
    args::Positional<std::string> input(required, "input", "Input file to process");
    args::Flag     print(parser, "print", "Print a summary of the file and exit.", {'p', "print"});
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::CompletionFlag completion(parser, {"complete"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Completion& e) {
        std::cout << e.what();
        return EXIT_SUCCESS;
    } catch (const args::Help&) {
        std::cout << parser;
        return EXIT_SUCCESS;
    } catch (args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        parser.Help(std::cerr);
        return EXIT_FAILURE;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return EXIT_FAILURE;
    }

    fs::path inputPath = args::get(input);

    bool convert = inputPath.extension() == ".gltf";
    bool write = static_cast<bool>(output);

    if (write) {
        if (convert) {
            fs::path outputPath = args::get(output);
            if (fs::exists(inputPath))
                RTRConvertedFile(outputPath, inputPath);
            else {
                std::cerr << "Input file not found: " << inputPath << "\n";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Input is not a .gltf\n";
            return EXIT_FAILURE;
        }
    } else {
        App app;
        if (convert) {
            app.view(rtrtool::File(RTRConvertedMemory(inputPath)));
        } else {
            try {
                app.view(rtrtool::File(rtrtool::MappedFile(inputPath)));
            } catch (const rtrtool::MappedFile::Error& e) {
                std::cout << e.what() << std::endl;
                return EXIT_FAILURE;
            }
        }
        app.renderloop();
    }

    return EXIT_SUCCESS;
}