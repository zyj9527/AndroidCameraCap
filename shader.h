#ifndef SHADER_H
#define SHADER_H

#include <string>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#if 0
#include <cutils/properties.h>
#include <utils/Log.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif

class Shader {
public:
    // State
    GLuint ID;

    // Constructor
    Shader() {}

    // Sets the current shader as active
    Shader &Use();

    // Compiles the shader from given source code
    void Compile(const GLchar *vertexSource, const GLchar *fragmentSource,
                 const GLchar *geometrySource = 0); // Note: geometry source code is optional
    // Utility functions
    void SetFloat(const GLchar *name, GLfloat value, GLboolean useShader = false);

    void SetInteger(const GLchar *name, GLint value, GLboolean useShader = false);

    void SetVector2f(const GLchar *name, GLfloat x, GLfloat y, GLboolean useShader = false);

    void SetVector2f(const GLchar *name, const glm::vec2 &value, GLboolean useShader = false);

    void SetVector3f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLboolean useShader = false);

    void SetVector3f(const GLchar *name, const glm::vec3 &value, GLboolean useShader = false);

    void SetVector4f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLboolean useShader = false);

    void SetVector4f(const GLchar *name, const glm::vec4 &value, GLboolean useShader = false);

    void SetMatrix4(const GLchar *name, const glm::mat4 &matrix, GLboolean useShader = false);

private:
    void checkCompileErrors(GLuint object, char *type);
};

#endif
