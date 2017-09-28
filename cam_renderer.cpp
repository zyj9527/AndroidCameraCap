#include "cam_renderer.h"

CamRenderer::CamRenderer(Shader &shader) {
    this->shader = shader;
    this->initRenderData();
}

CamRenderer::~CamRenderer() {
    glDeleteVertexArraysOES(1, &this->quadVAO);
}

void CamRenderer::DrawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size, GLfloat rotate) {
    // Prepare transformations
    this->shader.Use();
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(position,
                                            0.0f));  // First translate (transformations are: scale happens first, then rotation and then finall translation happens; reversed order)

    if (rotate != 0) {
        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y,
                                                0.0f)); // Move origin of rotation to center of quad
        model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f)); // Then rotate
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // Move origin back
    }

    model = glm::scale(model, glm::vec3(size, 1.0f)); // Last scale

    this->shader.SetMatrix4("model", model);

    // Render textured quad
    //this->shader.SetVector3f("spriteColor", color);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArrayOES(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArrayOES(0);
}

void CamRenderer::initRenderData() {
    // Configure VAO/VBO
    GLuint VBO;
    GLfloat vertices[] = {
            // Pos      // Tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,

            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArraysOES(1, &this->quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArrayOES(this->quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArrayOES(0);
}
