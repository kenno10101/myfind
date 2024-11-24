#include <iostream>
#include <filesystem> // for directory iterator
#include <string>
#include <vector>        // for storing childprocesses and waiting
#include <unistd.h>      // für getopt()
#include <dirent.h>      // für directory operations
#include <sys/wait.h>    // for waitpid
#include <bits/stdc++.h> // for transform function/lowercase function
#include <semaphore.h>   // for synchronisation of processes
#include <fcntl.h>       //needed for processes

namespace fs = std::filesystem;

class Myfind
{
public:
    static bool directory_find(
        const fs::path &path,
        std::string &filename,
        bool is_recursive,
        bool is_case_insensitive);

private:
    static std::string getFilenameFromDirEntry(fs::directory_entry const &dir_entry);
    static fs::path getAbsolutePathFromDirEntry(fs::directory_entry const &dir_entry);
    static void convert_tolower(std::string &input);
};