
#include "shader_s.h"

#pragma once
class renderer {

protected:
    unsigned int VBO = 0, VAO = 0, EBO = 0;
    unsigned int indexCount;

    glm::mat4 modelMatrix;

    Shader* myShader;

public: void setXForm(glm::mat4 mat)
{
    modelMatrix = mat;
}

public: void rotate(float axis[], float angle)
{
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(axis[0], axis[1], axis[2]));
}    

public: void translate(float trans[])
{
    modelMatrix = glm::translate(modelMatrix, glm::vec3(trans[0], trans[1], trans[2]));
}

public: void scale(float scale[])
{
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale[0], scale[1], scale[2]));
}

    public:  void render(glm::mat4 vMat, glm::mat4 pMat)
    { // here's where the "actual drawing" gets done

        myShader->use();

        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "v"), 1, GL_FALSE, glm::value_ptr(vMat));
        glUniformMatrix4fv(glGetUniformLocation(myShader->ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));


        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
};