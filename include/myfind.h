#ifndef MYFIND_H
#define MYFIND_H

#include <iostream>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <string>
#include <dirent.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <algorithm>

namespace fs = std::filesystem;

class Myfind
{
public:
    static bool directory_find(
        const fs::path &path,
        const std::string &filename,
        bool is_recursive,
        bool is_case_insensitive,
        std::vector<std::pair<std::string, fs::path>> &found_files);

    static void convert_tolower(std::string &input);
};

#endif
