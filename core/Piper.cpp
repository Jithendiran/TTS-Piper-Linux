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
        if (byteswrite >= 0) return byteswrite;
        else if(byteswrite == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return 0;
        else {
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
    if(can_read()) {
        size_t bytesread = ::read(op_pipe[0], text_data, len);
        if(bytesread >= 0) return bytesread;
        else if(bytesread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return 0;
        else {
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
            if(n > 0) continue;
            else if (n == 0) break;
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
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
            if(n > 0) continue;
            else if (n == 0) break;
            else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
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

int main() {
     const char *tts_args[] = {
        "--config",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json",
        "--output-raw", "--json-input", "--sentence-silence 0.1", "--length_scale 1.2",
        NULL};
    Piper *tts = new Piper(
        "/opt/calibre/bin/piper/piper",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx",
        tts_args,
        7, true);
    char *text[] = {
        "{\"text\": \"Hi hello i am super man.\"}\n", 
        "{\"text\": \"Hi hello you  man.\"}\n", 
        "{\"text\": \"A week ago a friend invited a couple of other couples over for dinner. Eventually, the food (but not the wine) was cleared off the table for what turned out to be some fierce Scrabbling. Heeding the strategy of going for the shorter, more valuable word over the longer cheaper word, our final play was Bon, which–as luck would have it!–happens to be a Japanese Buddhist festival, and not, as I had originally asserted while laying the tiles on the board, one half of a chocolate-covered cherry treat. Anyway, the strategy worked. My team only lost by 53 points instead of 58.\"}\n",
        // "{\"text\": \"Hi hello you  man.\"}\n", 
        NULL};
    
        char **add = text;
    char buff[5000] = {0};
    tts->init();
    int retries = 100;
    while (!tts->is_started() && retries-- > 0)
    {
        usleep(10000); // 10ms
    }
    while(*add) {
        cout<< "playing :  "<<*add;

        ssize_t n = tts->write(*add);
        cout << "Total len : " << strlen(*add) << " Written : " << n << endl;        
        ssize_t r = 0;
        while(true) {
            r = tts->read(buff,sizeof(buff));
            if(r>0)
            cout << buff << endl;
            memset(buff, 0, r);
            if(r <= 0 && tts->is_completed()){
                break;
            }
        }

        cout << "completed \n";
        // signal done
        *add++;
    }
    return 0;
}

// g++ -g -I./include Piper.cpp -o Piper.o