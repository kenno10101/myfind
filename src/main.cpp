#include "../include/myfind.h"

void print_usage(const std::string &program_name); // function to print usage and exit
void take_args(int argc, char *argv[], bool &is_recursive, bool &is_case_insensitive); // function to handle arguments
void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive, sem_t *sem); // function to handle child process
void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath, sem_t *sem); // function to handle child processes
fs::path validate_searchpath(const std::string &searchpath); // function to validate the searchpath

/*
File search is handled by:
1. Parsing arguments to get filenames, search path, and optional flags (-R for recursive, -i for case-insensitive search)
2. Validating the search path (does it exist?, is it accessible?,  is it a directory?)
3. Forking a child process for each filename to search within the provided path and optionally in subdirectories
4. Creating a semaphore for synchronization and a clean output of the search results (unsorted)
5. Printing results to stdout in the given format
*/

int main(int argc, char *argv[])
{
    // flags for the -R (recursive) and -i (case insensitive) options
    bool is_recursive = false;
    bool is_case_insensitive = false;

    take_args(argc, argv, is_recursive, is_case_insensitive);

    fs::path searchpath = validate_searchpath(argv[optind++]);

    // create or open a semaphore for synchronization
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

// print usage instructions if input is incorrect
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

    while ((c = getopt(argc, argv, "Ri")) != EOF) // searches for options
    {
        switch (c)
        {
        case 'R':
            if (is_recursive) // check if -R has already been set
            {
                error = true;
            }
            is_recursive = true;
            break;
        case 'i':
            if (is_case_insensitive) // check if -i has already been set
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

    // print error message if there is an error or arguments missing
    if (error || optind >= argc || argc == optind + 1)
    {
        print_usage(argv[0]);
    }
}

// check if path exists or of it is a directory
fs::path validate_searchpath(const std::string &searchpath)
{
    if (!fs::exists(searchpath) || !fs::is_directory(searchpath)) 
    {
        std::cerr << "Error: " << searchpath << " is not a valid directory.\n";
        exit(EXIT_FAILURE);
    }
    return searchpath;
}

// child process function to search for the file in the given directory
void child(const fs::path &searchpath, const std::string &filename, bool is_recursive, bool is_case_insensitive, sem_t *sem)
{
    DIR *dirp = opendir(searchpath.c_str());
    if (dirp == nullptr)
    {
        std::cerr << "Error: Could not open directory " << searchpath << "\n";
        exit(EXIT_FAILURE);
    }

    // Lock semaphore before accessing shared list
    // sync the output using the semaphore
    if (sem_wait(sem) == -1)
    {
        std::cerr << "Semaphore wait failed in child process." << std::endl;
        exit(EXIT_FAILURE);
    }

    bool file_found = Myfind::directory_find(const_cast<fs::path &>(searchpath), const_cast<std::string &>(filename), is_recursive, is_case_insensitive);

    if (!file_found)
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

// handle child processes for each file search
void handle_childprocesses(int &argc, char *argv[], bool &is_recursive, bool &is_case_insensitive, fs::path &searchpath, sem_t *sem)
{
    std::vector<pid_t> children;

    for (int i = optind; i < argc; i++)
    { // opind points to the first argument
        std::string filename = argv[i];
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            child(searchpath, filename, is_recursive, is_case_insensitive, sem);
        }

        else if (pid > 0)
        {
            // Parent process
            children.push_back(pid);
        }

        else // fork failed
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