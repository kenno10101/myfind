#include "../include/myfind.h"

void print_usage(const std::string &program_name);
void take_args(int argc, char *argv[], bool &is_recursive, bool &is_case_insensitive);
void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive);
void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath);
fs::path validate_searchpath(const std::string &searchpath);

int main(int argc, char *argv[])
{
    bool is_recursive = false;
    bool is_case_insensitive = false;

    take_args(argc, argv, is_recursive, is_case_insensitive);

    fs::path searchpath = validate_searchpath(argv[optind++]);

    handle_childprocesses(argc, argv, is_recursive, is_case_insensitive, searchpath);

    return EXIT_SUCCESS;
}

// Ausgabe einer Fehlermeldung bei ungültiger Eingabe
void print_usage(const std::string &program_name)
{
    std::cerr << "Usage: " << program_name << " [-R] [-i] searchpath filename1 [filename2] ... [filenameN]\n";
    exit(EXIT_FAILURE);
}

// handle arguments
void take_args(int argc, char *argv[], bool &is_recursive, bool &is_case_insensitive)
{
    int c;
    bool error = false;

    while ((c = getopt(argc, argv, "Ri")) != EOF) // durchsucht die Argumente nach Ri
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

fs::path validate_searchpath(const std::string &searchpath)
{
    if (!fs::exists(searchpath) || !fs::is_directory(searchpath)) // überprüft ob der Pfad existiert und ob es ein Verzeichnis ist
    {
        std::cerr << "Error: " << searchpath << " is not a valid directory.\n";
        exit(EXIT_FAILURE);
    }
    return searchpath;
}

void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive)
{
    DIR *dirp = opendir(searchpath.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "Error: Could not open directory " << searchpath << "\n";
        exit(EXIT_FAILURE);
    }

    bool file_found = Myfind::directory_find(const_cast<fs::path &>(searchpath), const_cast<std::string &>(filename), is_recursive, is_case_insensitive);
    closedir(dirp);

    if (!file_found)
    {
        std::cout << "File " << filename << " not found in " << searchpath << "\n";
    }

    exit(EXIT_SUCCESS);
}

void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath)
{
    std::vector<pid_t> children;

    for (int i = optind; i < argc; i++)
    { // opind zeigt jetzt auf das erste Argument nach den Optionen
        std::string filename = argv[i];
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            child(searchpath, filename, is_recursive, is_case_insensitive);
        }

        else if (pid > 0)
        {
            // Parent process
            children.push_back(pid);
        }

        else
        {
            std::cerr << "Error: Could not create child process\n";
            exit(EXIT_FAILURE);
        }
    }

    // wartet auf alle childprozesse
    for (pid_t pid : children)
    {
        int status;
        pid_t wpid = waitpid(pid, &status, 0);

        if (wpid == -1)
        {
            std::cerr << "Error: waitpid failed\n";
            exit(EXIT_FAILURE);
        }
    }
}

// Todo : kommentieren
// Todo : nicht 2x das gleiche opt
// Todo : Bei mehreren Dateien sollen alle gefundenen Dateien ausgegeben werden