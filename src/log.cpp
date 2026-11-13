#include "log.h"
#include "typedefs.h"

#include <string.h>

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

    // NOTE: snprintf/vsnprintf return the length the string would have had, not what was written,
    // so CharCount has to be clamped, otherwise a long message moves the write pointer past the buffer
    char Buffer[4096];
    i32 MaxCount = (i32)sizeof(Buffer) - 2; // Leaves room for the newline written to the log file

    // Local time of day, with milliseconds so logs within the same frame can still be told apart
    SDL_Time Now = 0;
    SDL_DateTime Time = {};
    SDL_GetCurrentTime(&Now);
    SDL_TimeToDateTime(Now, &Time, true);

    i32 CharCount = snprintf(Buffer, sizeof(Buffer), "%s[%02d:%02d:%02d.%03d] [%s]: ", Color,
                             Time.hour, Time.minute, Time.second, Time.nanosecond / 1000000, CategoryString);
    if(CharCount < 0) CharCount = 0;
    if(CharCount > MaxCount) CharCount = MaxCount;

    i32 MessageCount = vsnprintf(Buffer + CharCount, sizeof(Buffer) - CharCount, Fmt, Args);
    if(MessageCount > 0) CharCount += MessageCount;
    if(CharCount > MaxCount) CharCount = MaxCount;

    snprintf(Buffer + CharCount, sizeof(Buffer) - CharCount, "%s", ResetColorCode);

    va_end(Args);

    // The first log of the run opens the file, the previous run's log is renamed with the date it was last written
    static SDL_IOStream *File = nullptr;
    static b32 FileOpened = false;
    if(!FileOpened)
    {
        FileOpened = true; // Only try once, if it fails we keep logging to the console
        SDL_CreateDirectory("logs");

        SDL_PathInfo Info;
        if(SDL_GetPathInfo("logs/scoundrel.txt", &Info))
        {
            SDL_DateTime Date;
            SDL_TimeToDateTime(Info.modify_time, &Date, true); // Local time
            char OldPath[64];
            snprintf(OldPath, sizeof(OldPath), "logs/scoundrel_%04d-%02d-%02d-%02d-%02d-%02d.txt",
                     Date.year, Date.month, Date.day, Date.hour, Date.minute, Date.second);
            SDL_RenamePath("logs/scoundrel.txt", OldPath);
        }

        File = SDL_IOFromFile("logs/scoundrel.txt", "w");
    }

    if(File)
    {
        // Same line as the console without the terminal color codes, one write per line
        size_t ColorLength = strlen(Color);
        char *Line = Buffer + ColorLength;
        size_t LineLength = CharCount - ColorLength;
        char Saved = Line[LineLength];
        Line[LineLength] = '\n';
        SDL_WriteIO(File, Line, LineLength + 1);
        Line[LineLength] = Saved; // Put back the start of the reset color code for the console

        // NOTE: On Windows SDL_FlushIO waits for the disk (FlushFileBuffers), only pay that for errors
        if(Category == Error) SDL_FlushIO(File);
    }

    puts(Buffer);
}
#endif
