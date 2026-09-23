#include "helpers.h"
#include "typedefs.h"

const char *FilenameFromPath(const char *Filepath)
{
    // Get the last occurence of "/" || "\\", add 1 to it to get the filename

#ifdef _WIN32
    const char *Filename = strrchr(Filepath, '\\');
#else
    const char *Filename = strrchr(Filepath, '/');
#endif

    return Filename ? Filename + 1 : nullptr;
}

b32 FileExists(const char *Filepath)
{
    return SDL_GetPathInfo(Filepath, NULL);
}
