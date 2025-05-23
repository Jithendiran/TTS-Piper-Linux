#ifndef PIPER_HPP
#define PIPER_HPP
#include "core.hpp"
class Piper : public Itts {
    int argslen = 0;
    bool is_async = false;
    bool is_ready = false;
    bool is_processed = false;
    char **args;
    char *pgm_path;
    char *model_path;

    int check_for_word(char *para, char *word) const;

    public:
    Piper() = delete;
    Piper(const char *pgm_path, const char *model_path, const char *var_param[], int var_param_count, bool is_async);
    bool init();
    bool can_read();
    bool is_started();
    ssize_t write(const char *text_data);
    ssize_t read(char *text_data, ssize_t len);
    bool is_completed();
    bool interrupt();
    ~Piper();
};
#endif