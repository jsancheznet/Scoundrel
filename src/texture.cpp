#include "texture.h"
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>

texture CreateTexture(const char* File)
{
    texture Result;

    i32 ChannelCount;

    stbi_set_flip_vertically_on_load(true);

    u8 *ImageData = stbi_load(File, &Result.Width, &Result.Height, &ChannelCount, 4);

    glCreateTextures(GL_TEXTURE_2D, 1, &Result.Id);

    glTextureParameteri(Result.Id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Result.Id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Result.Id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(Result.Id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(Result.Id, 1, GL_RGBA8, Result.Width, Result.Height);

    glTextureSubImage2D(Result.Id, 0, 0, 0, Result.Width, Result.Height, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);

    glGenerateTextureMipmap(Result.Id);

    stbi_image_free(ImageData);

    Log(Info, "CreateTexture() - Successfully created texture, %s with Id %d", File, Result.Id);

    return Result;
}
