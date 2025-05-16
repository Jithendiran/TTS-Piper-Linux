/*
This program can take maximum of 600 char as input
Input format "This is example string"

*/

// g++ TTSabstract.cpp -c TTSabstract.o
// g++ TTSabstract.cpp -o TTSabstract.o
#include <cstring>
#include <spawn.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <signal.h>
#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

class Process_c
{
private:
    int pid = -1;

protected:
    bool is_ready = false;

public:
    bool check_is_process_alive()
    {
        // Check if the process is alive
        if (pid > 0)
        {
            if (kill(pid, 0) == -1)
            {
                if (errno == ESRCH)
                {
                    cout << "Process does not exist." << endl;
                }
                else if (errno == EPERM)
                {
                    cout << "No permission to send signal to the process." << endl;
                }
                return false;
            }
            return true;
        }
        else
        {
            cout << "Invalid PID. Process is not running." << endl;
            return false;
        }
    }
    virtual bool interrupt() = 0;

    int getPid() { return pid; }

    virtual void setPid(int pid) { this->pid = pid; }
    // This function will pause the piper
    virtual bool pause()
    {
        if (check_is_process_alive())
        {
            if (-1 == kill(pid, SIGSTOP))
            {
                return false;
            }
            return true;
        }
        return false;
    }

    // This function will reume the piper
    virtual bool play()
    {
        if (check_is_process_alive())
        {
            if (-1 == kill(pid, SIGCONT))
            {
                return false;
            }
            return true;
        }
        return false;
    }

    // This function will terminate the piper process
    virtual bool stop()
    {
        if (check_is_process_alive())
        {
            // close the pipe
            if (-1 == kill(pid, SIGTERM))
            {
                return false;
            }

            int status;
            if (waitpid(pid, &status, 0) == -1)
            {
                perror("Failed to wait for process termination");
                return false;
            }
        }
        pid = -1;
        is_ready = false;
        return true;
    }
    virtual ~Process_c()
    {
        pid = -1;
        is_ready = false;
    }
};
class TTS : public Process_c
{
public:
    virtual bool init() = 0;
    virtual bool can_read() = 0;
    virtual bool is_started() = 0;
    virtual bool is_completed() = 0;
    virtual bool process_text(const char *text_data) = 0;
    virtual ssize_t get_raw_audiodata(char *text_data, ssize_t len) = 0;
    virtual ~TTS() {};
};

class Audio : public Process_c
{
public:
    virtual bool init() = 0;
    virtual bool is_started() = 0;
    virtual bool can_write_audio() = 0;
    virtual ssize_t write_raw(const char *buffer, ssize_t len) = 0;
    virtual ~Audio() {};
};

class ALSA : public Audio
{
    char **args;
    char *pgm_path;
    int argslen = 0;
    int audio_ip_pipe[2];  // write in parent 1, read in child 0
    int audio_op_pipe[2];  // write in child 1, read in parent 0
    int audio_err_pipe[2]; // write in child 1, read in parent 0
    posix_spawn_file_actions_t action_audio;

    void close_pipes()
    {
        if (audio_ip_pipe[1] >= 0)
        {
            close(audio_ip_pipe[1]);
            audio_ip_pipe[1] = -1;
        }

        if (audio_op_pipe[0] >= 0)
        {
            close(audio_op_pipe[0]);
            audio_op_pipe[0] = -1;
        }

        if (audio_err_pipe[0] >= 0)
        {
            close(audio_err_pipe[0]);
            audio_err_pipe[0] = -1;
        }
    }

public:
    ALSA() = delete;
    ALSA(const char *pgm_path, const char *var_param[], int var_param_count = 0)
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
    }
    bool init()
    {

        int pid;
        posix_spawn_file_actions_init(&action_audio);

        if (pipe(audio_ip_pipe) || pipe(audio_op_pipe) || pipe(audio_err_pipe))
        {
            perror("pipe");
            return false;
        }

        // child close
        // closing read end for child
        if (posix_spawn_file_actions_addclose(&action_audio, audio_op_pipe[0]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action_audio, audio_err_pipe[0]) != 0)
        {
            return false;
        }
        // closing write end for child
        if (posix_spawn_file_actions_addclose(&action_audio, audio_ip_pipe[1]) != 0)
        {
            return false;
        }
        //-------------------------------------------------------------------

        // redirections
        // redirecting the stdin of child to the write end of pipe
        if (posix_spawn_file_actions_adddup2(&action_audio, audio_ip_pipe[0], STDIN_FILENO) != 0)
        {
            return false;
        }
        // redirecting the stdout of child to the read end of pipe
        if (posix_spawn_file_actions_adddup2(&action_audio, audio_op_pipe[1], STDOUT_FILENO) != 0)
        {
            return false;
        }
        // redirecting the stderr of child to the read end of pipe
        if (posix_spawn_file_actions_adddup2(&action_audio, audio_err_pipe[1], STDERR_FILENO) != 0)
        {
            return false;
        }
        //-------------------------------------------------------------------

        // closing the
        // after redirecting the pipes, we can close the file actions
        if (posix_spawn_file_actions_addclose(&action_audio, audio_ip_pipe[0]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action_audio, audio_op_pipe[1]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action_audio, audio_err_pipe[1]) != 0)
        {
            return false;
        }

        extern char **environ;
        int status = posix_spawnp(&pid, pgm_path, &action_audio, NULL, args, environ);
        setPid(pid);

        if (status != 0)
        {
            perror("posix_spawnp");
            // free mem
            return false;
        }
        // closing the write end of pipe in parent
        close(audio_ip_pipe[0]);
        close(audio_op_pipe[1]);
        close(audio_err_pipe[1]);

        // fcntl(tts_op_pipe[0], F_SETFL, O_NONBLOCK);
        // fcntl(tts_err_pipe[0], F_SETFL, O_NONBLOCK);
        is_started();
        return true;
    }

    bool can_write_audio()
    {
        fd_set fds;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(audio_ip_pipe[1], &fds);
        return select(audio_ip_pipe[1] + 1, NULL, &fds, NULL, &timeout) > 0 && FD_ISSET(audio_ip_pipe[1], &fds);
    }

    bool is_started()
    {
        // check audio_ip_pipe is taking input

        return can_write_audio();
    }

    virtual ssize_t write_raw(const char *buffer, ssize_t len)
    {
        // wat happen if buffer is full and not completely written
        if (buffer)
        {
            while (!can_write_audio())
            {
                usleep(100000);
            }

            ssize_t bytesWrite = write(audio_ip_pipe[1], buffer, len);
            return bytesWrite;
        }
        return 0;
    }

    virtual bool interrupt()
    {
        // char buffer[1000];
        // fflush(audio_ip_pipe[1]);
        // fsync(fileno(audio_ip_pipe[1]));  // Optional but helps
        stop();
        return true;
    }
    bool stop()
    {
        cout << "In stop " << endl;
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
                kill(pid, SIGKILL);       // force kill
                waitpid(pid, &status, 0); // ensure zombie cleanup
            }
        }

        argslen = 0;
        close_pipes();
        posix_spawn_file_actions_destroy(&action_audio);
        if (pgm_path)
        {
            delete[] pgm_path;
            pgm_path = nullptr;
        }
        if (args)
        {
            for (int i = 0; i < argslen - 1; i++)
            {
                if (args[i])
                    delete[] args[i];
            }
            delete[] args;
            args = nullptr;
        }

        return Process_c::stop();
    }

    ~ALSA()
    {
        // pid = -1;
       
    }
};

class Piper : public TTS
{
    char **args;
    char *pgm_path;
    char *model_path;
    int argslen = 0;
    bool is_processed = false;
    // bool is_started = false;
    int tts_ip_pipe[2];  // write in parent 1, read in child 0
    int tts_op_pipe[2];  // read in parent 0, write in child 1
    int tts_err_pipe[2]; // read in parent 0, write in child 1
    posix_spawn_file_actions_t action;

    int check_for_word(char *para, char *word)
    {
        if (strlen(para) && strlen(word) && strstr(para, word))
        {
            return 1;
        }
        return 0;
    }

public:
    Piper() = delete;
    Piper(const char *pgm_path, const char *model_path, const char *var_param[], int var_param_count = 1)
    {
        // throw error if pgm_path, model_path is not valid

        int pgm_path_len = strlen(pgm_path) + 1;
        int model_path_len = strlen(model_path) + 1;

        this->pgm_path = new char[pgm_path_len];
        strcpy(this->pgm_path, pgm_path);

        this->model_path = new char[model_path_len];
        strcpy(this->model_path, model_path);

        argslen = var_param_count + 3;
        args = new char *[argslen]; // why 3? 0-> path, 1-> --model, 2-> model_path, last -> null

        // path of pgm
        args[0] = new char[pgm_path_len];
        strcpy(args[0], pgm_path);

        // model flag
        char *flag = "--model";
        args[1] = new char[strlen(flag) + 1];
        strcpy(args[1], flag);

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

        if (pipe(tts_ip_pipe) || pipe(tts_op_pipe) || pipe(tts_err_pipe))
        {
            perror("pipe");
            return false;
        }

        // child close
        // closing read end for child
        if (posix_spawn_file_actions_addclose(&action, tts_op_pipe[0]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action, tts_err_pipe[0]) != 0)
        {
            return false;
        }
        // closing write end for child
        if (posix_spawn_file_actions_addclose(&action, tts_ip_pipe[1]) != 0)
        {
            return false;
        }
        //-------------------------------------------------------------------

        // redirections
        // redirecting the stdin of child to the write end of pipe
        if (posix_spawn_file_actions_adddup2(&action, tts_ip_pipe[0], STDIN_FILENO) != 0)
        {
            return false;
        }
        // redirecting the stdout of child to the read end of pipe
        if (posix_spawn_file_actions_adddup2(&action, tts_op_pipe[1], STDOUT_FILENO) != 0)
        {
            return false;
        }
        // redirecting the stderr of child to the read end of pipe
        if (posix_spawn_file_actions_adddup2(&action, tts_err_pipe[1], STDERR_FILENO) != 0)
        {
            return false;
        }
        //-------------------------------------------------------------------

        // closing the
        // after redirecting the pipes, we can close the file actions
        if (posix_spawn_file_actions_addclose(&action, tts_ip_pipe[0]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action, tts_op_pipe[1]) != 0)
        {
            return false;
        }
        if (posix_spawn_file_actions_addclose(&action, tts_err_pipe[1]) != 0)
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
        close(tts_ip_pipe[0]);
        close(tts_op_pipe[1]);
        close(tts_err_pipe[1]);

        // fcntl(tts_op_pipe[0], F_SETFL, O_NONBLOCK);
        // fcntl(tts_err_pipe[0], F_SETFL, O_NONBLOCK);
        is_started();
        return true;
    }

    bool can_read()
    {
        fd_set read_fds;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        FD_ZERO(&read_fds);
        FD_SET(tts_op_pipe[0], &read_fds);
        return select(tts_op_pipe[0] + 1, &read_fds, NULL, NULL, &timeout) > 0 && FD_ISSET(tts_op_pipe[0], &read_fds);
    }

    // This function will return true when piper started accepting input
    bool is_started()
    {
        if (is_ready)
            return true;
        char buffererr[1024];
        ssize_t bytesReaderr = read(tts_err_pipe[0], buffererr, sizeof(buffererr) - 1);
        if (bytesReaderr == 0)
        {
            return false;
        }
        buffererr[bytesReaderr] = '\0';
        is_ready = check_for_word(buffererr, "Initialized piper") == 1 ? true : false;
        return is_ready;
    }

    bool process_text(const char *text_data)
    {
        int len = strlen(text_data);
        if (text_data)
        {
            size_t byteswrite = write(tts_ip_pipe[1], text_data, strlen(text_data));
            cout << "Process : " << len << " " << byteswrite << endl;
            return len == byteswrite;
        }
        return false;
    }

    ssize_t get_raw_audiodata(char *text_data, ssize_t len)
    {
        if (len <= 0)
        {
            return 0; // Invalid buffer or length
        }
        if (can_read())
        {
            return read(tts_op_pipe[0], text_data, len);
        }
        return 0 ;
    }

    bool is_completed()
    {
        char buffererr[1024];
        ssize_t bytesReaderr = read(tts_err_pipe[0], buffererr, sizeof(buffererr) - 1);
        if (bytesReaderr == 0)
        {
            return false;
        }
        buffererr[bytesReaderr] = '\0';

        return check_for_word(buffererr, "Waiting for audio to finish playing");
    }

    bool interrupt()
    {
        char buffer[1000];
        // based on the performance drain err, ip
        while (tts_op_pipe[0])
        {
            if (can_read())
            {
                read(tts_op_pipe[0], buffer, sizeof(buffer));
                continue;
            }
            else
            {
                return true;
            }
        }
        return true;
    }

    ~Piper()
    {
        argslen = 0;
        if (tts_ip_pipe[1] >= 0)
        {
            close(tts_ip_pipe[1]);
        }

        if (tts_op_pipe[0] >= 0)
        {
            close(tts_op_pipe[0]);
        }

        if (tts_err_pipe[0] >= 0)
        {
            close(tts_err_pipe[0]);
        }

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
class TTSController
{
    Piper *tts;
    ALSA *audio;
    bool is_interrupt = false;

public:
    TTSController(Piper *ttsEngine, ALSA *audioEngine)
        : tts(ttsEngine), audio(audioEngine) {}

    bool start()
    {
        if (!tts->init())
            return false;

        if (!audio->init()) {
            cout << "Audio init issue" << endl;
            return false;
        }
        cout << "Audio init completed" << endl;
        if (!audio->is_started()) {
            cout << "Audio start issue" << endl;
            return false;
        }

        // Wait until TTS is ready
        int retries = 100;
        while (!tts->is_started() && retries-- > 0)
        {
            usleep(10000); // 10ms
        }
        if (!tts->is_started())
            return false;

        return true;
    }

    bool write(const char *text)
    {
        if (is_interrupted()){
            return false;
        }
        return tts->process_text(text);
    }

    void streamAudio()
    {
        cout << "stream started........" << endl;
        // if (!audio->init()) {
        //     cout << "Audio init issue" << endl;
        //     return ;
        // }
        // cout << "Audio init completed" << endl;
        // if (!audio->is_started()) {
        //     cout << "Audio start issue" << endl;
        //     return ;
        // }

        // cout << "Audio start completed" << endl;

        while (!tts->can_read())
        {
            usleep(1000);
        }

         cout << "Piper ready" << endl;

        char buffer[20000];

        while (tts->can_read())
        {
            // cout<<"Can read" << endl;
            if (is_interrupted())
            {
                interrupt();
                break;
            }
            size_t bytes_read = tts->get_raw_audiodata(buffer, sizeof(buffer));

            // cout << "Piper read : "<<bytes_read <<endl;
            size_t bytes_write = audio->write_raw(buffer, bytes_read);
            // cout << "Bytes write : " << bytes_write <<endl;
            
            memset(buffer, 0, sizeof(buffer));

            if (!tts->can_read() && bytes_read == 0 && tts->is_completed()) {
                cout << " IS complete" << endl;
                break;
            }
        }
        cout << "stream completed........" << endl;
    }

    bool is_completed()
    {
        if (audio->can_write_audio())
        {
            usleep(2000000); // for current audio to complete
            return true;
        }
        return false;
    }

    void set_interrupt(bool is_interrupt)
    {
        this->is_interrupt = is_interrupt;
    }

    bool is_interrupted()
    {
        return is_interrupt;
    }

    void interrupt()
    {
        tts->interrupt();
        audio->interrupt();
    }

    void pause()
    {
        tts->pause();
        audio->pause();
    }

    void resume()
    {
        tts->play();
        audio->play();
    }

    void stop()
    {
        tts->stop();
        audio->stop();
    }
};

int main()
{
    const char *tts_args[] = {
        "--config",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json",
        "--output-raw", "--json-input", "--sentence-silence 0.1", "--length_scale 1.2",
        NULL};
    char *text[] = {
        "{\"text\": \"Hi hello i am super man.\"}\n", 
        "{\"text\": \"Hi hello you  man.\"}\n", 
        // "{\"text\": \"A week ago a friend invited a couple of other couples over for dinner. Eventually, the food (but not the wine) was cleared off the table for what turned out to be some fierce Scrabbling. Heeding the strategy of going for the shorter, more valuable word over the longer cheaper word, our final play was Bon, which–as luck would have it!–happens to be a Japanese Buddhist festival, and not, as I had originally asserted while laying the tiles on the board, one half of a chocolate-covered cherry treat. Anyway, the strategy worked. My team only lost by 53 points instead of 58.\"}\n",
        // "{\"text\": \"Hi hello you  man.\"}\n", 
        NULL};
    Piper *tts = new Piper(
        "/opt/calibre/bin/piper/piper",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx",
        tts_args,
        7);
    const char *alsa_args[] = {"-f", "S16_LE", "-r", "22050", "-q", NULL};
    ALSA *audio = new ALSA("aplay", alsa_args, 5);

    TTSController controller(tts, audio);
    controller.start();
    char **add = text;
    while(*add) {
        cout<< "playing :  "<<*add;

        controller.write(*add);
        controller.streamAudio();
        while (!controller.is_completed())
        {
        }
        cout << "completed \n";
        // signal done
        *add++;
    }
    controller.stop();
    
    delete tts;
    delete audio;
    return 0;
}


/*
/opt/calibre/bin/piper/piper --model /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx --config /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json --output-raw --json-input --sentence-silence 0.1 --length_scale 1.2

*/