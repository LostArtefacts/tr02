#include "include.h"
#include "global.h"
#include <eh.h>
#include "se_exception.h"

extern "C" void SE_TranslateException(unsigned int code, EXCEPTION_POINTERS *pointers)
{
    throw SE_Exception(code);
}
