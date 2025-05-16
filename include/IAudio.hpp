#ifndef IPROCESS_C
#define IPROCESS_C
#include "IProcess_c.hpp"
#endif

class IAudio : public IProcess_c
{
public:
    virtual bool is_started() = 0;
    virtual bool can_write_audio() = 0;
    virtual ssize_t write(const char *buffer, ssize_t len) = 0;
    virtual ~IAudio() {};
};