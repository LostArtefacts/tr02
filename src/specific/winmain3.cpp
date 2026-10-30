#include "include.h"
#include "global.h"
#include <eh.h>

class SE_Exception {
public:
    SE_Exception(unsigned int code) : code(code) {}
    ~SE_Exception() {}

private:
    unsigned int code;
};

extern "C" void Func44e920(unsigned int code, EXCEPTION_POINTERS *pointers)
{
    throw SE_Exception(code);
}
