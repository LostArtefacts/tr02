#include "include.h"
#include "global.h"
#include <eh.h>
#include "se_exception.h"

extern "C" void Func44e920(unsigned int code, EXCEPTION_POINTERS *pointers)
{
    throw SE_Exception(code);
}
