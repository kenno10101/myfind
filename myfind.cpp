#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h> // für getopt()
#include <dirent.h> // für directory operations
#include <vector>
#include <string>
#include <sys/wait.h> // for waitpid

namespace fs = std::filesystem;


// Ausgabe der Meldung
void print_usage(const std::string &program_name)
{
    std::cerr << "Usage: " << program_name << " [-R] [-i] searchpath filename1 [filename2] ... [filenameN]\n";
    exit(EXIT_FAILURE);
}

// In verzeichnis nach datei suchen
bool directory_find(fs::path &path, std::string &filename, bool is_recursive, bool is_case_insensitive)
{
    if (is_case_insensitive) // case insensitive
    {
        filename = filename;
    }

    if (is_recursive) // rekursive suche
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
    else // nicht rekursive suche
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

void take_args(int argc, char *argv[], bool &is_recursive, bool &is_case_insensitive)
{
    int c;
    bool error = false;

    while ((c = getopt(argc, argv, "Ri")) != -1)
    {
        switch (c)
        {
        case 'R':
            is_recursive = true;
            break;
        case 'i':
            is_case_insensitive = true;
            break;
        default:
            error = true;
        }
    }

    if (error || optind >= argc)
    {
        print_usage(argv[0]);
    }
}

void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive)
{
    DIR *dirp = opendir(searchpath.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "Error: Could not open directory " << searchpath << "\n";
        exit(EXIT_FAILURE);
    }

    bool file_found = directory_find(const_cast<fs::path &>(searchpath), const_cast<std::string &>(filename), is_recursive, is_case_insensitive);
    closedir(dirp);

    if (!file_found)
    {
        std::cout << "File " << filename << " not found in " << searchpath << "\n";
    }

    exit(EXIT_SUCCESS);
}

fs::path validate_searchpath(const std::string &searchpath)
{
    if (!fs::exists(searchpath) || !fs::is_directory(searchpath))
    {
        std::cerr << "Error: " << searchpath << " is not a valid directory.\n";
        exit(EXIT_FAILURE);
    }
    return searchpath;
}

void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath)
{
    std::vector<pid_t> children;

    for (int i = optind; i < argc; i++) {
        std::string filename = argv[i];
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            child(searchpath, filename, is_recursive, is_case_insensitive);
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
}


int main(int argc, char *argv[])
{
    bool is_recursive = false;
    bool is_case_insensitive = false;

    take_args(argc, argv, is_recursive, is_case_insensitive);

    fs::path searchpath = validate_searchpath(argv[optind++]);

    handle_childprocesses(argc, argv, is_recursive, is_case_insensitive, searchpath);

    return EXIT_SUCCESS;
}
