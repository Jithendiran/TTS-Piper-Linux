#ifndef IPROCESS_C
#define IPROCESS_C
#include "IProcess_c.hpp"
#endif

class Itts : public IProcess_c
{
public:
    virtual bool can_read() = 0;
    virtual bool is_started() = 0;
    virtual bool is_completed() = 0;
    virtual bool write(const char *text_data) = 0;
    virtual ssize_t read(char *text_data, ssize_t len) = 0;
    virtual ~Itts() {};
};