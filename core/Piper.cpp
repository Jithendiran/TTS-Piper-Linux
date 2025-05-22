#include "Piper.hpp"

int Piper::check_for_word(char *para, char *word) const
{
    if (para && word && strlen(para) && strlen(word) && strstr(para, word))
    {
        return 1;
    }
    return 0;
}

Piper::Piper(const char *pgm_path, const char *model_path, const char *var_param[], int var_param_count = 1, bool is_async = false)
{
    int pgm_path_len = strlen(pgm_path) + 1;
    int model_path_len = strlen(model_path) + 1;

    this->pgm_path = new char[pgm_path_len];
    strcpy(this->pgm_path, pgm_path);

    this->model_path = new char[model_path_len];
    strcpy(this->model_path, model_path);

    argslen = var_param_count + 3; // why 3? 0-> path, 1-> --model, 2-> model_path, last -> null
    args = new char *[argslen];

    // path of pgm
    args[0] = new char[pgm_path_len];
    strcpy(args[0], pgm_path);

    // model flag
    string flag = "--model";
    args[1] = new char[flag.length() + 1];
    strcpy(args[1], flag.c_str());

    // model path
    args[2] = new char[model_path_len];
    strcpy(args[2], model_path);

    if (var_param && argslen > 0)
    {
        for (int i = 0; i < argslen - 3; i++)
        {
            int index = i + 3;
            if (var_param[i])
            {
                args[index] = new char[strlen(var_param[i]) + 1];
                strcpy(this->args[index], var_param[i]);
            }
            else
            {
                args[index] = nullptr;
            }
        }
    }
    this->is_async = is_async;
}

bool Piper::init()
{
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return false;
    }

    if (pid == 0)
    {
        // child
        // when parent dies, send child SIGTERM for termination
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        // child close
        // closing read end for child
        if (!close_pipe(op_pipe[0]))
        {
            return false;
        }
        if (!close_pipe(err_pipe[0]))
        {
            return false;
        }
        // closing write end for child
        if (!close_pipe(ip_pipe[1]))
        {
            return false;
        }
        //-------------------------------------------------------------------

        // redirections
        // redirecting the stdin of child to the write end of pipe
        if (dup2(ip_pipe[0], STDIN_FILENO) == -1)
        {
            perror("dup2");
            return false;
        }
        // redirecting the stdout of child to the read end of pipe
        if (dup2(op_pipe[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            return false;
        }
        // redirecting the stderr of child to the read end of pipe
        if (dup2(err_pipe[1], STDERR_FILENO) == -1)
        {
            perror("dup2");
            return false;
        }
        //-------------------------------------------------------------------

        // closing the
        // after redirecting the pipes, we can close the file actions
        if (!close_pipe(ip_pipe[0]))
        {
            return false;
        }
        if (!close_pipe(op_pipe[1]))
        {
            return false;
        }
        if (!close_pipe(err_pipe[1]))
        {
            return false;
        }

        // exec
        execvp(pgm_path, args);
        perror("execvp"); // If exec fails
        _exit(1);
    }

    setPid(pid);
    cout << "Piper :: " << pid << endl;

    // closing the write end of pipe in parent
    close_pipe(ip_pipe[0]);
    close_pipe(op_pipe[1]);
    close_pipe(err_pipe[1]);

    setAsync(is_async);

    is_started();
    return true;
}

bool Piper::can_read()
{
    return can_read_pipe(op_pipe[0]);
}

bool Piper::is_started()
{
    if (is_ready)
        return true;
    if (!can_read_pipe(err_pipe[0]))
        return false;
    char buffererr[1024];
    ssize_t bytesReaderr = ::read(err_pipe[0], buffererr, sizeof(buffererr) - 1);
    if (bytesReaderr == 0)
    {
        return false;
    }
    if (bytesReaderr < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Non-blocking: no data available right now
            return false;
        }
        // Other error
        perror("read");
        return false;
    }

    buffererr[bytesReaderr] = '\0';
    string word = "Initialized piper";

    is_ready = check_for_word(buffererr, (char *)word.c_str()) == 1 ? true : false;
    return is_ready;
}

ssize_t Piper::write(const char *text_data)
{
    int len = strlen(text_data);
    if (text_data)
    {
        size_t byteswrite = ::write(ip_pipe[1], text_data, strlen(text_data));
        cout << "Process : " << len << " " << byteswrite << endl;
        if (byteswrite >= 0)
            return byteswrite;
        else if (byteswrite == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return 0;
        else
        {
            perror("write");
        }
    }
    return 0;
}

ssize_t Piper::read(char *text_data, ssize_t len)
{
    if (len <= 0 & text_data == NULL)
    {
        return 0; // Invalid buffer or length
    }
    if (can_read())
    {
        size_t bytesread = ::read(op_pipe[0], text_data, len);
        if (bytesread >= 0)
            return bytesread;
        else if (bytesread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return 0;
        else
        {
            perror("write");
        }
    }
    return 0;
}

bool Piper::is_completed()
{
    if (can_read_pipe(err_pipe[0]))
    {
        char buffererr[1024];
        ssize_t bytesReaderr = ::read(err_pipe[0], buffererr, sizeof(buffererr) - 1);
        if (bytesReaderr == 0)
        {
            return false;
        }
        else if (bytesReaderr == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Non-blocking: no data available right now
                return false;
            }
            // Other error
            perror("read");
            return false;
        }
        buffererr[bytesReaderr] = '\0';
        string word = "Waiting for audio to finish playing";
        return check_for_word(buffererr, (char *)word.c_str());
    }
    return false;
}

bool Piper::interrupt()
{
    char buffer[1000];
    ssize_t n;
    while (op_pipe[0] > 0)
    {
        if (can_read())
        {
            n = ::read(op_pipe[0], buffer, sizeof(buffer));
            if (n > 0)
                continue;
            else if (n == 0)
                break;
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                perror("read");
                break;
            }
        }
        else
        {
            return true;
        }
    }
    n = 0;
    while (err_pipe[0] > 0)
    {
        if (can_read_pipe(err_pipe[0]))
        {
            n = ::read(err_pipe[0], buffer, sizeof(buffer));
            if (n > 0)
                continue;
            else if (n == 0)
                break;
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                perror("read");
                break;
            }
        }
        else
        {
            return true;
        }
    }

    return true;
}

Piper::~Piper()
{
    is_ready = false;
    is_processed = false;

    close_pipe(ip_pipe[1]);

    close_pipe(op_pipe[0]);

    close_pipe(err_pipe[0]);

    if (pgm_path)
    {
        delete[] pgm_path;
        pgm_path = nullptr;
    }
    if (model_path)
    {
        delete[] model_path;
        model_path = nullptr;
    }

    if (args)
    {
        for (int i = 0; i < argslen; i++)
        {
            if (args[i])
            {
                delete[] args[i];
                args[i] = nullptr;
            }
        }
        delete[] args;
        args = nullptr;
    }
    argslen = 0;
}

// clear && g++ -g -I./include Piper.cpp -o Piper.o && valgrind -s --leak-check=full ./Piper.o