#include <cxxopts.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        cxxopts::Options options("cellSim", "Simple CLI");
        options.add_options()
            ("n,name", "Name to greet", cxxopts::value<std::string>()->default_value("world"))
            ("h,help", "Show help");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        std::string name = result["name"].as<std::string>();
        std::cout << "Hello, " << name << "!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}