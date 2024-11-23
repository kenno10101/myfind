#include "../include/myfind.h"

void print_usage(const std::string &program_name);
void take_args(int argc, char *argv[], bool &is_recursive, bool &is_case_insensitive);
void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive, std::unordered_map<fs::path, int> &list_found_filepaths, sem_t *sem);
void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath, sem_t *sem);
fs::path validate_searchpath(const std::string &searchpath);

int main(int argc, char *argv[])
{
    bool is_recursive = false;
    bool is_case_insensitive = false;

    take_args(argc, argv, is_recursive, is_case_insensitive);

    fs::path searchpath = validate_searchpath(argv[optind++]);

    // Create or open a semaphore for synchronization
    sem_t *sem;
    sem_unlink("sync_semaphore"); // remove any existing semaphore before a new one
    sem = sem_open("sync_semaphore", O_CREAT | O_EXCL, 0644, 1);

    // if semaphore creation for any reason failed
    if (sem == SEM_FAILED)
    {
        std::cerr << "Semaphore creation failed. Exiting." << std::endl;
        perror("sem_open");
        return 1;
    }

    handle_childprocesses(argc, argv, is_recursive, is_case_insensitive, searchpath, sem);

    // close and unlink the semaphore
    sem_close(sem);
    sem_unlink("sync_semaphore");

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
            if (is_recursive) // Check if -R has already been set
            {
                error = true;
            }
            is_recursive = true;
            break;
        case 'i':
            if (is_case_insensitive) // Check if -i has already been set
            {
                error = true;
            }
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

void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive, std::unordered_map<fs::path, int> &list_found_filepaths, sem_t *sem)
{
    DIR *dirp = opendir(searchpath.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "Error: Could not open directory " << searchpath << "\n";
        exit(EXIT_FAILURE);
    }

    std::string output_filename = "";
    fs::path output_filepath = "";

    bool file_found = Myfind::directory_find(const_cast<fs::path &>(searchpath), const_cast<std::string &>(filename), is_recursive, is_case_insensitive, list_found_filepaths, output_filename, output_filepath);

    // Lock semaphore before accessing shared list
    // sync the output using the semaphore
    if (sem_wait(sem) == -1)
    {
        std::cerr << "Semaphore wait failed in child process." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (file_found)
    {
        std::cout << getpid() << ": " << output_filename << ": " << output_filepath << "\n";
    }
    else
    {
        std::cout << "File " << filename << " not found in " << searchpath << "\n";
    }
    // release the semaphore
    if (sem_post(sem) == -1)
    {
        std::cerr << "Semaphore post failed in child process." << std::endl;
        exit(EXIT_FAILURE);
    }

    closedir(dirp);
    exit(EXIT_SUCCESS);
}

void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath, sem_t *sem)
{
    std::vector<pid_t> children;

    // use unordered_map to count occurence of a filepath -> skip already found files
    std::unordered_map<fs::path, int> list_found_filepaths;

    for (int i = optind; i < argc; i++)
    { // opind zeigt jetzt auf das erste Argument nach den Optionen
        std::string filename = argv[i];
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            child(searchpath, filename, is_recursive, is_case_insensitive, list_found_filepaths, sem);
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

    // waits for all child processes
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
// Todo : Bei mehreren Dateien sollen alle gefundenen Dateien ausgegeben werden