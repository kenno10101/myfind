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
bool Myfind::directory_find(const fs::path &path, std::string &filename, bool is_recursive, bool is_case_insensitive)
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
                std::cout << getpid() << ": " << Myfind::getFilenameFromDirEntry(dir_entry) << ": " << Myfind::getAbsolutePathFromDirEntry(dir_entry) << "\n";
                file_found = true;
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
                std::cout << getpid() << ": " << dir_entry.path().filename() << ": " << Myfind::getAbsolutePathFromDirEntry(dir_entry) << "\n";
                file_found = true;
                break;
            }
        }
    }

    return file_found;
}
