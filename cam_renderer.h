#ifndef CAM_RENDERER_H
#define CAM_RENDERER_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "shader.h"

class CamRenderer
{
public:
    // Constructor (inits shaders/shapes)
    CamRenderer(Shader &shader);
    // Destructor
    ~CamRenderer();
    // Renders a defined quad textured with given sprite
    void DrawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f);
private:
    // Render state
    Shader shader; 
    GLuint quadVAO;
    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData();
};
#endif

