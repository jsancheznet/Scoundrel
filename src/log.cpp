#include "log.h"

void Log(log_category Category, const char *Fmt, ...)
{
    va_list Args;
    va_start(Args, Fmt);

    const char *CategoryString;
    const char *Color = "";
    const char *ResetColorCode = "\x1b[0m";
    switch(Category)
    {
        case Info:
        {
            CategoryString = "INFO";
        } break;
        case Success:
        {
            CategoryString = "SUCCESS";
            Color = "\x1b[32m";
        } break;
        case Warning:
        {
            CategoryString = "WARNING";
            Color = "\x1b[33m";
        } break;
        case Error:
        {
            CategoryString = "ERROR";
            Color = "\x1b[31m";
        } break;
        default:
        {
            CategoryString = "INVALID CATEGORY";
        }
    }

    //
    //     1- Write ColorString + CategoryString to buffer
    //     2- Write The Actual string
    //     3- Write the reset terminal color code
    //

    char Buffer[2048];
    int CharCount = snprintf(Buffer, sizeof(Buffer), "%s[%s]: ", Color, CategoryString);
    CharCount += vsnprintf(Buffer + CharCount, sizeof(Buffer) - CharCount, Fmt, Args);
    snprintf(Buffer + CharCount, sizeof(Buffer) - CharCount, "%s", ResetColorCode);

    va_end(Args);

    puts(Buffer);
}
