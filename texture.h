#ifndef TEXTURE_H
#define TEXTURE_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class Texture2D
{
public:
	GLenum Target;
    GLuint ID;
    GLuint Width, Height; // Width and height of loaded image in pixels
    GLuint Internal_Format; // Format of texture object
    GLuint Image_Format; // Format of loaded image
    GLuint Wrap_S; // Wrapping mode on S axis
    GLuint Wrap_T; // Wrapping mode on T axis
    GLuint Filter_Min; // Filtering mode if texture pixels < screen pixels
    GLuint Filter_Max; // Filtering mode if texture pixels > screen pixels
    Texture2D();
    Texture2D(GLenum target);
    void Generate(GLuint width, GLuint height, unsigned char* data);
    void Generate();
    void Bind() const;
};

#endif

