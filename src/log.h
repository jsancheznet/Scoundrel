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

void Log(log_category Category, const char *Fmt, ...);
