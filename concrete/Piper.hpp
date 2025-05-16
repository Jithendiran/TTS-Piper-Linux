#ifndef PIPER_HPP
#define PIPER_HPP
#include "concrete.hpp"
class Piper : public Itts {
    public:
    Piper(const char *pgm_path, const char *model_path, const char *var_param[], int var_param_count = 1);
    bool init();
    bool can_read();
    bool is_started();
    bool write(const char *text_data);
    ssize_t read(char *text_data, ssize_t len);
    bool is_completed();
    bool interrupt();
};
#endif