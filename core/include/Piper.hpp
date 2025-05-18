#ifndef PIPER_HPP
#define PIPER_HPP
#include "core.hpp"
class Piper : public Itts {
    char **args;
    char *pgm_path;
    char *model_path;
    int argslen = 0;
    bool is_processed = false;
    bool is_ready = false;

    posix_spawn_file_actions_t action;
    int check_for_word(char *para, char *word) const;

    public:
    Piper() = delete;
    Piper(const char *pgm_path, const char *model_path, const char *var_param[], int var_param_count);
    bool init();
    bool can_read();
    bool is_started();
    bool write(const char *text_data);
    ssize_t read(char *text_data, ssize_t len);
    bool is_completed();
    bool interrupt();
    ~Piper();
};
#endif