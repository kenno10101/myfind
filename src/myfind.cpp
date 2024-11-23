#include "../include/myfind.h"

void Myfind::convert_tolower(std::string &input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
}

std::string Myfind::getFilenameFromDirEntry(fs::directory_entry const &dir_entry)
{
    return dir_entry.path().filename();
}

fs::path Myfind::getAbsolutePathFromDirEntry(fs::directory_entry const &dir_entry)
{
    fs::path cwd = fs::current_path().parent_path();
    fs::path abs_path = fs::absolute(dir_entry.path());
    return fs::relative(abs_path, cwd);
}

// In verzeichnis nach datei suchen
bool Myfind::directory_find(const fs::path &path, std::string &filename, bool is_recursive, bool is_case_insensitive, std::unordered_map<fs::path, int> &list_found_filepaths, std::string &output_filename, fs::path &output_filepath)
{
    bool file_found = false;

    if (is_case_insensitive) // konvertiert in lowercase falls Option ausgewählt ist
    {
        Myfind::convert_tolower(filename);
    }

    if (is_recursive) // rekursive suche
    {
        for (fs::directory_entry const &dir_entry : fs::recursive_directory_iterator{path})
        {
            std::string entry_filename = Myfind::getFilenameFromDirEntry(dir_entry);

            if (is_case_insensitive)
            {
                Myfind::convert_tolower(entry_filename);
            }

            if (filename == entry_filename)
            {
                fs::path filepath = Myfind::getAbsolutePathFromDirEntry(dir_entry);

                if (list_found_filepaths[filepath] >= 1)
                {
                    continue;
                }
                std::cout << "DEBUG " << list_found_filepaths[filepath] << std::endl;
                list_found_filepaths[filepath]++;
                std::cout << "DEBUG " << list_found_filepaths[filepath] << std::endl;

                file_found = true;
                output_filename = Myfind::getFilenameFromDirEntry(dir_entry);
                output_filepath = filepath;
                break;
            }
        }
    }
    else // nicht rekursive suche
    {
        for (fs::directory_entry const &dir_entry : fs::directory_iterator{path})
        {
            std::string entry_filename = dir_entry.path().filename();

            if (is_case_insensitive)
            {
                Myfind::convert_tolower(entry_filename);
            }

            if (filename == entry_filename)
            {
                fs::path filepath = Myfind::getAbsolutePathFromDirEntry(dir_entry);

                if (list_found_filepaths[filepath] >= 1)
                {
                    continue;
                }
                list_found_filepaths[filepath]++;

                file_found = true;
                output_filename = Myfind::getFilenameFromDirEntry(dir_entry);
                output_filepath = filepath;
                break;
            }
        }
    }

    return file_found;
}
