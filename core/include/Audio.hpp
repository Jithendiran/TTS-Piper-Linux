#ifndef AUDIO_HPP
#define AUDIO_HPP
#include "core.hpp"
class Audio : public IAudio
{
    posix_spawn_file_actions_t action_audio;
    int argslen = 0;
    char **args;
    char *pgm_path;
    bool is_async = false;

    bool close_ip();
    bool close_op();
    bool close_err();
    void close_pipes();

public:
    Audio() = delete;
    Audio(const char *pgm_path, const char *var_param[], int var_param_count, bool is_async);
    bool init();
    bool is_started();
    bool can_write_audio();
    ssize_t write(const char *buffer, ssize_t len);
    bool interrupt();
    bool stop();
    ~Audio();
};
#endif