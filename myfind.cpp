#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h> // für getopt()
#include <dirent.h> // für directory operations

namespace fs = std::filesystem;

void print_usage(const std::string &program_name) {
    std::cerr << "Usage: " << program_name << " [-R] [-i] searchpath filename1 [filename2] ... [filenameN]\n";
    exit(EXIT_FAILURE);
}

bool directory_find(DIR *dirp, std::string filename){

    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL){
        std::string current_file = direntp->d_name;
        if (current_file[0] == '.'){
            continue;
        }
        std::cout << current_file << std::endl;
        if (filename == current_file){
            return true;
        }
    }

    return false;
    
}

bool directory_find_recursive(DIR *dirp, std::string &filename){
//TODO implement recursive search / search sub-directories
    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL){
        std::string current_file = direntp->d_name;
        if (current_file[0] == '.'){
            continue;
        }

        if(current_file.rfind(".", 0))
        std::cout << current_file << std::endl;
        if (filename == current_file){
            return true;
        }
    }

    return false;

}

int main(int argc, char *argv[]) {
    bool is_recursive = false;
    bool is_case_insensitive = false;
    int c;
    bool error = false;
    std::string program_name = argv[0];
    bool file_found = false;

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

    if(error || optind >= argc) {
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
        std::cerr << "Error: filename is missing.\n";
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

    std::cout << "DEBUG opened directory" << std::endl << "reading contents of opened directory:" << std::endl << std::endl;

    if(is_case_insensitive){

    }


    if (is_recursive){
        file_found = directory_find_recursive(dirp, filename);
    } else {
        file_found = directory_find(dirp, filename);
    }
    
    if (file_found){
        std::cout << "DEBUG file was found in given directory" << std::endl;
    } else {
        std::cout << "DEBUG file does not exist in given directory" << std::endl;
    }
      
    // TODO: Child process for each filepath 


    return EXIT_SUCCESS;
}
