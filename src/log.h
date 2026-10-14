#pragma once

#include <stdio.h>
#include <stdarg.h>

enum log_category
{
    Info,
    Warning,
    Error,
    Success,
};

#if defined(NDEBUG)
    #define Log(...) ((void)0)
#else
    void Log(log_category Category, const char *Fmt, ...);
#endif
