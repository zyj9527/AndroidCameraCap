

/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>

// Instantiate static variables
std::map<std::string, Texture2D>    ResourceManager::Textures;
std::map<std::string, Shader>       ResourceManager::Shaders;

Shader ResourceManager::LoadShader(const GLchar *vShader, const GLchar *fShader, std::string name) {
    Shader shader;
    shader.Compile(vShader, fShader, 0);
    Shaders[name] = shader;
    return Shaders[name];
}

Shader &ResourceManager::GetShader(std::string name) {
    return Shaders[name];
}

Texture2D ResourceManager::LoadTextureExternal(std::string name) {
    Texture2D texture(GL_TEXTURE_EXTERNAL_OES);
    texture.Generate();
    Textures[name] = texture;
    return Textures[name];
}

Texture2D &ResourceManager::GetTexture(std::string name) {
    return Textures[name];
}

void ResourceManager::Clear() {
    // (Properly) delete all shaders
    for (std::map<std::string, Shader>::iterator it = Shaders.begin(); it != Shaders.end(); ++it)
        glDeleteProgram(it->second.ID);
    Shaders.clear();

    for (std::map<std::string, Texture2D>::iterator it = Textures.begin(); it != Textures.end(); ++it)
        glDeleteTextures(1, &it->second.ID);
    Textures.clear();
}
