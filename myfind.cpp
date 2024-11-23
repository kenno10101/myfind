#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h> // für getopt()
#include <dirent.h> // für directory operations
#include <vector>
#include <sys/wait.h> // for waitpid
#include <semaphore.h>
#include <cctype>
#include <algorithm>

namespace fs = std::filesystem;

sem_t sem; // Semaphore für synchronisierten Output

// Ausgabe einer Fehlermeldung bei ungültiger Eingabe
void print_usage(const std::string &program_name)
{
    std::cerr << "Usage: " << program_name << " [-R] [-i] searchpath filename1 [filename2] ... [filenameN]\n";
    exit(EXIT_FAILURE);
}

// Case-insensitive Vergleichsfunktion
bool case_insensitive_compare(const std::string &s1, const std::string &s2)
{
    if (s1.size() != s2.size())
        return false;

    return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(),
                      [](char c1, char c2)
                      { return std::tolower(c1) == std::tolower(c2); });
}

// In Verzeichnis nach Datei suchen
bool directory_find(fs::path &path, std::string &filename, bool is_recursive, bool is_case_insensitive)
{
    bool found = false;

    if (is_recursive) // rekursive suche
    {
        for (const auto &dir_entry : fs::recursive_directory_iterator{path})
        {
            std::string entry_name = dir_entry.path().filename().string();

            if ((is_case_insensitive && case_insensitive_compare(entry_name, filename)) ||
                (!is_case_insensitive && entry_name == filename))
            {
                sem_wait(&sem); // Semaphore für synchronisierten Output
                std::cout << getpid() << ": " << filename << ": " << fs::absolute(dir_entry.path()) << "\n";
                sem_post(&sem);
                found = true;
            }
        }
    }
    else // nicht rekursive suche
    {
        for (const auto &dir_entry : fs::directory_iterator{path})
        {
            std::string entry_name = dir_entry.path().filename().string();

            if ((is_case_insensitive && case_insensitive_compare(entry_name, filename)) ||
                (!is_case_insensitive && entry_name == filename))
            {
                sem_wait(&sem); // Semaphore für synchronisierten Output
                std::cout << getpid() << ": " << filename << ": " << fs::absolute(dir_entry.path()) << "\n";
                sem_post(&sem);
                found = true;
            }
        }
    }

    return found;
}

void take_args(int argc, char *argv[], bool &is_recursive, bool &is_case_insensitive)
{
    int c;
    bool r_set = false;
    bool i_set = false;

    while ((c = getopt(argc, argv, "Ri")) != EOF) // durchsucht die Argumente nach Ri
    {
        switch (c)
        {
        case 'R':
            if (r_set)
            {
                std::cerr << "Error: Option '-R' specified multiple times.\n";
                print_usage(argv[0]);
            }
            is_recursive = true;
            r_set = true;
            break;
        case 'i':
            if (i_set)
            {
                std::cerr << "Error: Option '-i' specified multiple times.\n";
                print_usage(argv[0]);
            }
            is_case_insensitive = true;
            i_set = true;
            break;
        case '?':
        default:
            print_usage(argv[0]);
        }
    }

    if (optind >= argc)
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

    directory_find(const_cast<fs::path &>(searchpath), const_cast<std::string &>(filename), is_recursive, is_case_insensitive);
    closedir(dirp);

    exit(EXIT_SUCCESS);
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

void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath)
{
    std::vector<pid_t> children;

    for (int i = optind; i < argc; i++) { // opind zeigt jetzt auf das erste Argument nach den Optionen
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
            std::cerr << "Error: Could not create child process\n";
            exit(EXIT_FAILURE);
        }
    }

    // wartet auf alle childprozesse 
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
    sem_init(&sem, 0, 1); // Initialisiere Semaphore

    bool is_recursive = false;
    bool is_case_insensitive = false;

    take_args(argc, argv, is_recursive, is_case_insensitive);

    fs::path searchpath = validate_searchpath(argv[optind++]);

    handle_childprocesses(argc, argv, is_recursive, is_case_insensitive, searchpath);

    sem_destroy(&sem); // Zerstöre Semaphore
    return EXIT_SUCCESS;
}
