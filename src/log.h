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
    // The format attribute makes the compiler check Fmt against the arguments, like it does for printf,
    // so a call like Log(Info, "%s") fails to build instead of crashing at runtime
    void Log(log_category Category, const char *Fmt, ...) __attribute__((format(printf, 2, 3)));
#endif
