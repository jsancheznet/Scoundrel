#include "helpers.h"
#include "typedefs.h"
#include "json.h"

#include <cstring>
#include <cstdlib>

const char *FilenameFromPath(const char *Filepath)
{
    const char *Filename = strrchr(Filepath, '/');

    return Filename ? Filename + 1 : nullptr;
}

b32 FileExists(const char *Filepath)
{
    return SDL_GetPathInfo(Filepath, NULL);
}

void PrintMat4(const glm::mat4& m)
{
    for (int row = 0; row < 4; ++row)
    {
        std::printf(
            "[ %.3f %.3f %.3f %.3f ]\n",
            m[0][row],
            m[1][row],
            m[2][row],
            m[3][row]
        );
    }
}

json_value_s *JsonFind(json_object_s *Object, const char *Key)
{
    if(!Object) return NULL;

    for(json_object_element_s *Element = Object->start; Element; Element = Element->next)
    {
        if(std::strcmp(Element->name->string, Key) == 0)
        {
            return Element->value;
        }
    }

    return NULL;
}

json_object_s *JsonGetObject(json_object_s *Object, const char *Key)
{
    json_value_s *Value = JsonFind(Object, Key);
    return Value ? json_value_as_object(Value) : NULL;
}

json_array_s *JsonGetArray(json_object_s *Object, const char *Key)
{
    json_value_s *Value = JsonFind(Object, Key);
    return Value ? json_value_as_array(Value) : NULL;
}

f64 JsonGetNumber(json_object_s *Object, const char *Key, f64 Default)
{
    // json.h stores numbers as text, so they have to be converted
    json_value_s *Value = JsonFind(Object, Key);
    json_number_s *Number = Value ? json_value_as_number(Value) : NULL;
    return Number ? std::strtod(Number->number, NULL) : Default;
}

const char *JsonGetString(json_object_s *Object, const char *Key)
{
    json_value_s *Value = JsonFind(Object, Key);
    json_string_s *String = Value ? json_value_as_string(Value) : NULL;
    return String ? String->string : NULL;
}
