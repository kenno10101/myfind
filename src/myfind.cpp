#include "../include/myfind.h"

void Myfind::convert_tolower(std::string &input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
}

bool Myfind::directory_find(
    const fs::path &path,
    const std::string &filename,
    bool is_recursive,
    bool is_case_insensitive,
    std::vector<std::pair<std::string, fs::path>> &found_files)
{
    DIR *dirp = opendir(path.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "Error: Could not open directory " << path << std::endl;
        return false;
    }

    struct dirent *entry;
    bool file_found = false;

    while ((entry = readdir(dirp)) != nullptr)
    {
        std::string entry_name = entry->d_name;

        if (entry_name == "." || entry_name == "..")
        {
            continue;
        }

        std::string entry_name_lower = entry_name;
        if (is_case_insensitive)
        {
            Myfind::convert_tolower(entry_name_lower);
        }

        fs::path entry_path = path / entry_name;
        if (entry->d_type == DT_DIR && is_recursive)
        {
            directory_find(entry_path, filename, is_recursive, is_case_insensitive, found_files);
        }
        else if (entry->d_type == DT_REG)
        {
            std::string filename_lower = filename;
            if (is_case_insensitive)
            {
                Myfind::convert_tolower(filename_lower);
            }

            if (entry_name_lower == filename_lower)
            {
                found_files.emplace_back(entry_name, fs::absolute(entry_path));
                file_found = true;
            }
        }
    }

    closedir(dirp);
    return file_found;
}
