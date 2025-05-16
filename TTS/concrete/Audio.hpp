#ifndef AUDIO_HPP
#define AUDIO_HPP
#include "concrete.hpp"
class Audio : public IAudio {
    public:
    Audio(const char *pgm_path, const char *var_param[], int var_param_count = 0);
    bool init();
    bool is_started();
    bool can_write_audio();
    ssize_t write(const char *buffer, ssize_t len);
    bool interrupt();
     bool stop();
};
#endif