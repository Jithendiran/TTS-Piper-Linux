#ifndef ITTS
#define ITTS 
#include "../include/ITTS.hpp"
#endif

#include "concrete.hpp"

class Piper : public Itts
{
    char **args;
    char *pgm_path;
    char *model_path;
    int argslen = 0;
    bool is_processed = false;
    bool is_ready = false;

    posix_spawn_file_actions_t action;

    int check_for_word(char *para, char *word) const
    {
        if (para && word && strlen(para) && strlen(word) && strstr(para, word))
        {
            return 1;
        }
        return 0;
    }

public:
    Piper() = delete;
    Piper(const char *pgm_path, const char *model_path, const char *var_param[], int var_param_count = 1)
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
    }
    /**
     * This function will create a new process for piper
     */
    bool init()
    {
        int pid;
        posix_spawn_file_actions_init(&action);

        // child close
        // closing read end for child
        if (posix_spawn_file_actions_addclose(&action, op_pipe[0]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action, err_pipe[0]) != 0)
        {
            return false;
        }
        // closing write end for child
        if (posix_spawn_file_actions_addclose(&action, ip_pipe[1]) != 0)
        {
            return false;
        }
        //-------------------------------------------------------------------

        // redirections
        // redirecting the stdin of child to the write end of pipe
        if (posix_spawn_file_actions_adddup2(&action, ip_pipe[0], STDIN_FILENO) != 0)
        {
            return false;
        }
        // redirecting the stdout of child to the read end of pipe
        if (posix_spawn_file_actions_adddup2(&action, op_pipe[1], STDOUT_FILENO) != 0)
        {
            return false;
        }
        // redirecting the stderr of child to the read end of pipe
        if (posix_spawn_file_actions_adddup2(&action, err_pipe[1], STDERR_FILENO) != 0)
        {
            return false;
        }
        //-------------------------------------------------------------------

        // closing the
        // after redirecting the pipes, we can close the file actions
        if (posix_spawn_file_actions_addclose(&action, ip_pipe[0]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action, op_pipe[1]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action, err_pipe[1]) != 0)
        {
            return false;
        }

        int status = posix_spawnp(&pid, pgm_path, &action, NULL, args, NULL);
        setPid(pid);

        if (status != 0)
        {
            perror("posix_spawnp");
            // free mem
            return false;
        }
        // closing the write end of pipe in parent
        close(ip_pipe[0]);
        close(op_pipe[1]);
        close(err_pipe[1]);

        // fcntl(op_pipe[0], F_SETFL, O_NONBLOCK);
        // fcntl(err_pipe[0], F_SETFL, O_NONBLOCK);
        is_started();
        return true;
    }

    bool can_read()
    {
        return can_read_pipe(op_pipe[0]);
    }

    // This function will return true when piper started accepting input
    bool is_started()
    {
        if (is_ready)
            return true;

        char buffererr[1024];
        ssize_t bytesReaderr = ::read(err_pipe[0], buffererr, sizeof(buffererr) - 1);
        if (bytesReaderr == 0)
        {
            return false;
        }
        buffererr[bytesReaderr] = '\0';
        string word = "Initialized piper";
        
        is_ready = check_for_word(buffererr, (char *)word.c_str()) == 1 ? true : false;
        return is_ready;
    }

    bool write(const char *text_data)
    {
        int len = strlen(text_data);
        if (text_data)
        {
            size_t byteswrite = ::write(ip_pipe[1], text_data, strlen(text_data));
            cout << "Process : " << len << " " << byteswrite << endl;
            return len == byteswrite;
        }
        return false;
    }

    ssize_t read(char *text_data, ssize_t len)
    {
        if (len <= 0 & text_data == NULL)
        {
            return 0; // Invalid buffer or length
        }
        return can_read() ? ::read(op_pipe[0], text_data, len) : 0;
    }

    bool is_completed()
    {
        if (can_read_pipe(err_pipe[0])) {
            char buffererr[1024];
            ssize_t bytesReaderr = ::read(err_pipe[0], buffererr, sizeof(buffererr) - 1);
            if (bytesReaderr == 0)
            {
                return false;
            }
            buffererr[bytesReaderr] = '\0';
            string word = "Waiting for audio to finish playing";
            return check_for_word(buffererr, (char *)word.c_str());
        }
        return false;
    }

    bool interrupt()
    {
        char buffer[1000];
        
        while (op_pipe[0] > 0)
        {
            if (can_read())
            {
                ::read(op_pipe[0], buffer, sizeof(buffer));
                continue;
            }
            else
            {
                return true;
            }
        }

        while(err_pipe[0] > 0) {
            if(can_read_pipe(err_pipe[0])){
                ::read(err_pipe[0], buffer, sizeof(buffer));
                continue;
            } else {return true;}
        }

        return true;
    }

    ~Piper()
    {
        argslen = 0;
        is_ready = false;
        is_processed = false;
       
        close_pipe(ip_pipe[1]);

        close_pipe(op_pipe[0]);

        close_pipe(err_pipe[0]);

        posix_spawn_file_actions_destroy(&action);
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
                    delete[] args[i];
                args[i] = nullptr;
            }
            delete[] args;
            args = nullptr;
        }
    }
};