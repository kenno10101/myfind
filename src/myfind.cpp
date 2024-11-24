#include "../include/myfind.h"

// converts string to lowercase
void Myfind::convert_tolower(std::string &input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
}

// returns filename from a directory_entry
std::string Myfind::getFilenameFromDirEntry(fs::directory_entry const &dir_entry)
{
    return dir_entry.path().filename();
}

// returns absolute path from a directory_entry
fs::path Myfind::getAbsolutePathFromDirEntry(fs::directory_entry const &dir_entry)
{
    fs::path cwd = fs::current_path();
    return fs::absolute(fs::relative(dir_entry.path(), cwd));
}

// searches for a file in a directory
bool Myfind::directory_find(const fs::path &path, std::string &filename, bool is_recursive, bool is_case_insensitive)
{
    bool file_found = false;

    if (is_case_insensitive) // konvertiert in lowercase falls Option ausgewählt ist
    {
        Myfind::convert_tolower(filename);
    }

    if (is_recursive) // recursive search in subdirectories
    {
        for (fs::directory_entry const &dir_entry : fs::recursive_directory_iterator{path})
        {
            std::string entry_filename = Myfind::getFilenameFromDirEntry(dir_entry);
            std::string output_filename = entry_filename;

            if (is_case_insensitive)
            {
                Myfind::convert_tolower(entry_filename);
            }

            if (filename == entry_filename)
            {
                fs::path output_filepath = Myfind::getAbsolutePathFromDirEntry(dir_entry);

                std::cout << getpid() << ": " << output_filename << ": " << output_filepath << std::endl;

                file_found = true;
            }
        }
    }
    else // not recursive search
    {
        for (fs::directory_entry const &dir_entry : fs::directory_iterator{path})
        {
            std::string entry_filename = dir_entry.path().filename();
            std::string output_filename = entry_filename;

            if (is_case_insensitive)
            {
                Myfind::convert_tolower(entry_filename);
            }

            if (filename == entry_filename)
            {
                fs::path output_filepath = Myfind::getAbsolutePathFromDirEntry(dir_entry);

                std::cout << getpid() << ": " << output_filename << ": " << output_filepath << std::endl;

                file_found = true;
                break;
            }
        }
    }

    return file_found;
}
