#include "Audio.hpp"

bool Audio::close_ip()
{
    return close_pipe(ip_pipe[1]);
}
bool Audio::close_op()
{
    return close_pipe(op_pipe[0]);
}
bool Audio::close_err()
{
    return close_pipe(err_pipe[0]);
}

void Audio::close_pipes()
{
    close_ip();
    close_op();
    close_err();
}

Audio::Audio(const char *pgm_path, const char *var_param[], int var_param_count = 0, bool is_async = false)
{
    int pgm_path_len = strlen(pgm_path) + 1;
    this->pgm_path = new char[pgm_path_len];
    strcpy(this->pgm_path, pgm_path);

    argslen = var_param_count + 1; // 1-> path of pgm
    args = new char *[argslen];
    // path of pgm
    args[0] = new char[pgm_path_len];
    strcpy(args[0], pgm_path);

    if (var_param && argslen > 1)
    {
        for (int i = 0; i < argslen - 1; i++)
        {
            int index = i + 1;
            if (var_param[i])
            {
                args[index] = new char[strlen(var_param[i]) + 1];
                strcpy(this->args[index], var_param[i]);
            }
            else
            {
                args[index] = NULL;
            }
        }
    }
    this->is_async = is_async;
}

bool Audio::init()
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
    cout << "Aplay : " << pid << endl;
    // closing the write end of pipe in parent
    close_pipe(ip_pipe[0]);
    close_pipe(op_pipe[1]);
    close_pipe(err_pipe[1]);

    setAsync(is_async);

    is_started();
    return true;
}

bool Audio::is_started() { return can_write_audio(); }

bool Audio::can_write_audio()
{
    return can_write_pipe(ip_pipe[1]);
}

ssize_t Audio::write(const char *buffer, ssize_t len)
{
    if (len <= 0 || buffer == nullptr)
    {
        return 0;
    }

    int retries = 50; // 5 seconds max (50 * 100ms)
    while (!can_write_audio() && retries-- > 0)
    {
        usleep(100000); // 100ms
    }

    if (retries <= 0)
    {
        std::cerr << "[Audio::write] Timeout: pipe not writable after 5 seconds\n";
        return 0;
    }

    ssize_t byteswritten = ::write(ip_pipe[1], buffer, len);
    if (byteswritten >= 0)
        return byteswritten;
    else if (byteswritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return 0;
    else
    {
        perror("write");
    }

    return 0;
}

bool Audio::interrupt()
{
    // char buffer[1000];
    // fflush(audio_ip_pipe[1]);
    // fsync(fileno(audio_ip_pipe[1]));  // Optional but helps
    // stop();
    // free pipe
    return true;
}

bool Audio::stop()
{
    int pid = getPid();
    if (check_is_process_alive())
    {
        close_pipes();
        // close the pipe
        int status;
        pid_t result;
        int retry = 10;
        while ((result = waitpid(pid, &status, WNOHANG)) == 0 && retry-- > 0)
        {
            usleep(100000); // wait 100ms
        }
        if (result == 0)
        {
            cout << "aplay did not exit, forcing kill\n";
            if (kill(pid, SIGKILL) == -1)
            {
                // force kill
                return false;
            }
            waitpid(pid, &status, 0); // ensure zombie cleanup
        }
    }
    return true;
}

Audio::~Audio()
{
    int pid = getPid();
    close_pipes();

    if (pgm_path)
    {
        delete[] pgm_path;
        pgm_path = nullptr;
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
    close_pipes();
}