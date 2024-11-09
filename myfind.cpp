#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h> // für getopt()
#include <dirent.h> // für directory operations

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

    DIR *dirp;
    dirp = opendir(argv[optind]);

    fs::path searchpath = argv[optind++];
    if (!fs::exists(searchpath) || !fs::is_directory(searchpath)) {
        std::cerr << "Error: " << searchpath << " is not a valid directory.\n";
        return EXIT_FAILURE;
    }

    std::string filename = argv[optind++];
    if (filename.length() == 0) {
        std::cerr << "Error: " << filename << " is missing.\n";
        print_usage(program_name);
        return EXIT_FAILURE;
    }


    // TODO: Implement the search logic here


    bool directory_open_fail = dirp == NULL;
    if (directory_open_fail)
    {
        perror("failed to open directory");
        return 1;
    }

    std::cout << "opened directory" << std::endl << "reading contents of opened directory:" << std::endl;
    
    struct dirent *direntp;
    bool file_found = false;
    while ((direntp = readdir(dirp)) != NULL){
        std::cout << direntp->d_name << std::endl;
        if (filename == direntp->d_name){
            file_found = true;
        }
    }

    if (file_found){
        std::cout << "file was found in given directory" << std::endl;
    } else {
        std::cout << "file does not exist in given directory" << std::endl;
    }
      
    
    // TODO: Child process for each filepath 


    return EXIT_SUCCESS;
}
