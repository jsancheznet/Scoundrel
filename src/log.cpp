#include "log.h"
#include "typedefs.h"

#ifndef NDEBUG
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

    // NOTE: snprintf/vsnprintf return the length the string would have had, not what was written,
    // so CharCount has to be clamped, otherwise a long message moves the write pointer past the buffer
    char Buffer[4096];
    i32 MaxCount = (i32)sizeof(Buffer) - 1;

    i32 CharCount = snprintf(Buffer, sizeof(Buffer), "%s[%s]: ", Color, CategoryString);
    if(CharCount < 0) CharCount = 0;
    if(CharCount > MaxCount) CharCount = MaxCount;

    i32 MessageCount = vsnprintf(Buffer + CharCount, sizeof(Buffer) - CharCount, Fmt, Args);
    if(MessageCount > 0) CharCount += MessageCount;
    if(CharCount > MaxCount) CharCount = MaxCount;

    snprintf(Buffer + CharCount, sizeof(Buffer) - CharCount, "%s", ResetColorCode);

    va_end(Args);

    puts(Buffer);
}
#endif
