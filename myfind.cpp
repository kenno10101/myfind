#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h> // für getopt()

namespace fs = std::filesystem;

void print_usage(const std::string &program_name) {
    std::cerr << "Usage: " << program_name << " [-R] [-i] searchpath filename1 [filename2] …[filenameN]\n";
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    bool is_recursive = false;
    bool is_case_insensitive = false;
    int c;
    bool error = false;
    std::string program_name = argv[0];

    while ((c = getopt(argc, argv, "Ri")) != EOF) {
        switch (c) {
            case 'R': is_recursive = true; 
            break;
            case 'i': is_case_insensitive = true; 
            break;
            case '?':
                error = true;
            default:
                error = true;
        }
    }

    if(error) {
        print_usage(program_name);
    }

    if (optind >= argc) {
        print_usage(program_name);
    }

    fs::path searchpath = argv[optind++];
    if (!fs::exists(searchpath) || !fs::is_directory(searchpath)) {
        std::cerr << "Error: " << searchpath << " is not a valid directory.\n";
        return EXIT_FAILURE;
    }

    // TODO: Implement the search logic here
    
    // TODO: Child process for each filepath 

    // TODO : Change Makefile


    return EXIT_SUCCESS;
}
