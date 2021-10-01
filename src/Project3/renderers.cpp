#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "renderer.h"
#include "ImportedModel.h"
#include "SceneGraph.h"

std::map<std::string, Material*> Material::materials;
std::map<std::string, Shader*> Shader::shaders;

std::vector<Renderer*> Renderer::renderList;

void Renderer::render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg)
{ // here's where the "actual drawing" gets done

    glm::mat4 mvp;

    if (!enabled) return;

    //assert(myMaterial != NULL);

    if (myMaterial == NULL)
        myMaterial = Material::materials["green"];

    glm::vec3 lightLoc = sg->light.position;
    glm::vec3 cameraLoc = sg->camera.position;

    myMaterial->use();

    //rotate(glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)), deltaTime); // easter egg!  rotate incrementally with delta time

    
    mvp = pMat * vMat * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(myMaterial->myShader->ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myMaterial->myShader->ID, "v"), 1, GL_FALSE, glm::value_ptr(vMat));
    glUniformMatrix4fv(glGetUniformLocation(myMaterial->myShader->ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));

    glUniform1f(glGetUniformLocation(myMaterial->myShader->ID, "myTime"), elapsedTime += (float)deltaTime);
    glUniform1f(glGetUniformLocation(myMaterial->myShader->ID, "shine"), myMaterial->shine);

    glUniformMatrix4fv(glGetUniformLocation(myMaterial->myShader->ID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glUniform3fv(glGetUniformLocation(myMaterial->myShader->ID, "cPos"), 1, glm::value_ptr(cameraLoc));
    glUniform3fv(glGetUniformLocation(myMaterial->myShader->ID, "lPos"), 1, glm::value_ptr(lightLoc));

    glm::mat4 lightViewProjection = sg->light.projection() * glm::lookAt(sg->light.position, sg->light.target, sg->light.up);
    
    glUniformMatrix4fv(glGetUniformLocation(myMaterial->myShader->ID,"lightSpaceMatrix"),1,GL_FALSE,glm::value_ptr(lightViewProjection));
    
    glBindVertexArray(VAO);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (indexCount < 0) {
        glDrawArrays(GL_TRIANGLES, 0, -indexCount);
    }
    else
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::setupColorAttrib() {
    // color attribute

    float color[3];

    if (myMaterial == NULL)
        myMaterial = Material::materials["green"];

    color[0] = glm::value_ptr(myMaterial->color)[0];
    color[1] = glm::value_ptr(myMaterial->color)[1];
    color[2] = glm::value_ptr(myMaterial->color)[2];

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), color, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
}
