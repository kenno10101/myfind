#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h> // für getopt()
#include <dirent.h> // für directory operations
#include <vector>
#include <string>
#include <sys/wait.h> // for waitpid

namespace fs = std::filesystem;

void print_usage(const std::string &program_name)
{
    std::cerr << "Usage: " << program_name << " [-R] [-i] searchpath filename1 [filename2] ... [filenameN]\n";
    exit(EXIT_FAILURE);
}

bool directory_find(fs::path &path, std::string &filename, bool is_recursive, bool is_case_insensitive)
{
    if (is_case_insensitive)
    {
        filename = filename;
    }

    if (is_recursive)
    {
        for (auto const &dir_entry : fs::recursive_directory_iterator{path})
        {
            if (filename == dir_entry.path().filename())
            {
                std::cout << getpid() << ": " << filename << ": " << dir_entry.path() << "\n";
                return true;
            }
        }
    }
    else
    {
        for (auto const &dir_entry : fs::directory_iterator{path})
        {
            if (filename == dir_entry.path().filename())
            {
                std::cout << getpid() << ": " << filename << ": " << dir_entry.path() << "\n";
                return true;
            }
        }
    }

    return false;
    
}

int main(int argc, char *argv[])
{
    bool is_recursive = false;
    bool is_case_insensitive = false;
    int c;
    bool error = false;
    std::string program_name = argv[0];
    bool file_found = false;

    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (c)
        {
        case 'R':
            is_recursive = true;
            break;
        case 'i':
            is_case_insensitive = true;
            break;
        case '?':
            error = true;
        default:
            error = true;
        }
    }

    if (error || optind >= argc)
    {
        print_usage(program_name);
    }

    DIR *dirp;
    dirp = opendir(argv[optind]);

    fs::path searchpath = argv[optind++];
    if (!fs::exists(searchpath) || !fs::is_directory(searchpath))
    {
        std::cerr << "Error: " << searchpath << " is not a valid directory.\n";
        return EXIT_FAILURE;
    }

    std::string filename = argv[optind++];
    if (filename.length() == 0)
    {
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

    // std::cout << "DEBUG opened directory" << std::endl
    //           << "reading contents of opened directory:" << std::endl
    //           << std::endl;

    file_found = directory_find(searchpath, filename, is_recursive, is_case_insensitive);

    if (!file_found)
    {
        std::cout << "DEBUG file does not exist in given directory" << std::endl;
    }
      
    // TODO: Child process for each filepath 

    std::vector<pid_t> children;

    for (int i = optind; i < argc; i++) {
        std::string filename = argv[i];
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            DIR *dirp = opendir(searchpath.c_str()); // convert fs::path to const char * and open directory
            if (dirp == NULL) {
                std::cerr << "Error: Could not open directory " << searchpath << std::endl;
                exit(EXIT_FAILURE);
            }

            if (is_recursive){
                file_found = directory_find_recursive(dirp, filename);
            } else {
                file_found = directory_find(dirp, filename);
            }

            closedir(dirp);

            if (!file_found) {
                std::cout << "File " << filename << " not found in " << searchpath << std::endl;
            }

            exit(EXIT_SUCCESS);
        } 
        
        else if (pid > 0) {
            // Parent process
            children.push_back(pid);
        } 
        
        else {
            // Error
            std::cerr << "Error: Could not create child process\n";
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all child processes to finish
    for (pid_t pid : children) {
        int status;
        pid_t wpid = waitpid(pid, &status, 0);

        if(wpid == -1) {
            std::cerr << "Error: waitpid failed\n";
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
