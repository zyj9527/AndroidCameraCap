#include <iostream>

#include "texture.h"

Texture2D::Texture2D()
        : Target(GL_TEXTURE_2D), Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT),
          Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR) {
    glGenTextures(1, &this->ID);
}

Texture2D::Texture2D(GLenum target)
        : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT),
          Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR) {
    this->Target = target;
    glGenTextures(1, &this->ID);
}

void Texture2D::Generate(GLuint width, GLuint height, unsigned char *data) {
    this->Width = width;
    this->Height = height;
    // Create Texture
    glBindTexture(Target, this->ID);
    glTexImage2D(Target, 0, this->Internal_Format, width, height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
    // Set Texture wrap and filter modes
    glTexParameteri(Target, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(Target, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    // Unbind texture
    glBindTexture(Target, 0);
}

void Texture2D::Generate() {
    glBindTexture(Target, this->ID);

    glTexParameterf(Target, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameterf(Target, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameterf(Target, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameterf(Target, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    glBindTexture(Target, 0);
}

void Texture2D::Bind() const {
    glBindTexture(Target, this->ID);
}
