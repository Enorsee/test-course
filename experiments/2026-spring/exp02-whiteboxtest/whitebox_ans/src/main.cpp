#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "analysis.h"
#include "cfg.h"
#include "parser.h"
#include "tokenizer.h"

namespace fs = std::filesystem;

namespace {

struct Options {
    std::string input_file;
    std::string function_name;
    std::string dot_file;
    std::string render_file;
    bool analysis{false};
    int max_visits{3};
};

std::string read_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open input file: " + path);
    }
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

void write_file(const std::string& path, const std::string& content) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Could not write file: " + path);
    }
    output << content;
}

void print_usage() {
    std::cerr
        << "Usage: cfg_lab [options] <input.c>\n"
        << "Options:\n"
        << "  --function NAME   Analyze the named function (default: first function)\n"
        << "  --dot FILE        Write the CFG as a Graphviz DOT file\n"
        << "  --render FILE     Render the CFG using dot (output format from extension)\n"
        << "  --analysis        Compute prime paths and selected test paths\n"
        << "  --max-visits N    Bound node visits when enumerating complete test paths\n";
}

Options parse_args(int argc, char* argv[]) {
    Options options;

    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--function") {
            if (index + 1 >= argc) {
                throw std::runtime_error("Missing value for --function");
            }
            options.function_name = argv[++index];
        } else if (arg == "--dot") {
            if (index + 1 >= argc) {
                throw std::runtime_error("Missing value for --dot");
            }
            options.dot_file = argv[++index];
        } else if (arg == "--render") {
            if (index + 1 >= argc) {
                throw std::runtime_error("Missing value for --render");
            }
            options.render_file = argv[++index];
        } else if (arg == "--analysis") {
            options.analysis = true;
        } else if (arg == "--max-visits") {
            if (index + 1 >= argc) {
                throw std::runtime_error("Missing value for --max-visits");
            }
            options.max_visits = std::stoi(argv[++index]);
        } else if (!arg.empty() && arg[0] == '-') {
            throw std::runtime_error("Unknown option: " + arg);
        } else if (options.input_file.empty()) {
            options.input_file = arg;
        } else {
            throw std::runtime_error("Unexpected extra argument: " + arg);
        }
    }

    if (options.input_file.empty()) {
        throw std::runtime_error("Missing input file");
    }
    return options;
}

const cfglab::Function& select_function(const cfglab::Program& program,
                                        const std::string& requested_name) {
    if (program.functions.empty()) {
        throw std::runtime_error("No function definitions were found");
    }
    if (requested_name.empty()) {
        return program.functions.front();
    }
    for (const auto& function : program.functions) {
        if (function.name == requested_name) {
            return function;
        }
    }
    throw std::runtime_error("Function not found: " + requested_name);
}

void render_with_graphviz(const std::string& dot_path, const std::string& output_path) {
    fs::path output(output_path);
    std::string format = output.extension().string();
    if (!format.empty() && format.front() == '.') {
        format.erase(format.begin());
    }
    if (format.empty()) {
        format = "svg";
    }

    const std::string command = "dot -T" + format + " \"" + dot_path + "\" -o \"" +
                                output_path + "\"";
    const int exit_code = std::system(command.c_str());
    if (exit_code != 0) {
        throw std::runtime_error("Graphviz rendering failed");
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        const Options options = parse_args(argc, argv);
        const std::string source = read_file(options.input_file);
        cfglab::Parser parser(cfglab::tokenize(source));
        const cfglab::Program program = parser.parse_program();
        const cfglab::Function& function =
            select_function(program, options.function_name);

        cfglab::CFGBuilder builder;
        const cfglab::CFG cfg = builder.build(function);

        std::cout << cfglab::cfg_to_text(cfg) << '\n';

        std::string dot_path = options.dot_file;
        if (!options.render_file.empty() && dot_path.empty()) {
            fs::path render_path(options.render_file);
            dot_path = render_path.replace_extension(".dot").string();
        }

        if (!dot_path.empty()) {
            write_file(dot_path, cfglab::cfg_to_dot(cfg));
            std::cout << "DOT file written to: " << dot_path << '\n';
        }

        if (!options.render_file.empty()) {
            render_with_graphviz(dot_path, options.render_file);
            std::cout << "Rendered graph written to: " << options.render_file << '\n';
        }

        if (options.analysis) {
            const cfglab::CoverageReport report =
                cfglab::compute_prime_path_coverage(cfg, options.max_visits);

            std::cout << "\nPrime Paths (" << report.prime_paths.size() << "):\n";
            for (std::size_t index = 0; index < report.prime_paths.size(); ++index) {
                std::cout << "  P" << index + 1 << ": "
                          << cfglab::path_to_string(report.prime_paths[index]) << '\n';
            }

            std::cout << "\nCandidate Complete Paths (" << report.candidate_test_paths.size()
                      << ", max_visits=" << report.max_visits << "):\n";
            for (std::size_t index = 0; index < report.candidate_test_paths.size(); ++index) {
                std::cout << "  C" << index + 1 << ": "
                          << cfglab::path_to_string(report.candidate_test_paths[index])
                          << '\n';
            }

            std::cout << "\nSelected Test Paths (" << report.selected_test_paths.size()
                      << "):\n";
            for (std::size_t index = 0; index < report.selected_test_paths.size(); ++index) {
                std::cout << "  T" << index + 1 << ": "
                          << cfglab::path_to_string(report.selected_test_paths[index])
                          << '\n';
            }
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        print_usage();
        return 1;
    }
}
