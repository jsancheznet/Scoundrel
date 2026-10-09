#pragma once

#include <glm/glm.hpp>
#include "typedefs.h"

struct json_value_s;
struct json_object_s;
struct json_array_s;

const char *FilenameFromPath(const char *Filepath);

b32 FileExists(const char *Filepath);
void PrintMat4(const glm::mat4& m);

// Json helpers. All of them return NULL (or Default) if the key is missing or the value has the wrong type.
json_value_s  *JsonFind(json_object_s *Object, const char *Key);
json_object_s *JsonGetObject(json_object_s *Object, const char *Key);
json_array_s  *JsonGetArray(json_object_s *Object, const char *Key);
f64            JsonGetNumber(json_object_s *Object, const char *Key, f64 Default = 0.0);
const char    *JsonGetString(json_object_s *Object, const char *Key);
