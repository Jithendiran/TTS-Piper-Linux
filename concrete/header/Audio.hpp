#ifndef AUDIO_HPP
#define AUDIO_HPP
#include "concrete.hpp"
class Audio : public IAudio
{
    char **args;
    char *pgm_path;
    int argslen = 0;
    posix_spawn_file_actions_t action_audio;

     bool close_ip();
     bool close_op();
     bool close_err();
     void close_pipes();

public:
Audio() = delete;
    Audio(const char *pgm_path, const char *var_param[], int var_param_count);
    bool init();
    bool is_started();
    bool can_write_audio();
    ssize_t write(const char *buffer, ssize_t len);
    bool interrupt();
    bool stop();
    ~Audio();
};
#endif