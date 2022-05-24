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

void Renderer::render(glm::mat4 treeMat, glm::mat4 vpMat, double deltaTime, SceneGraph *sg)
{ // here's where the "actual drawing" gets done

    /* you may be wondering what happened to the view and projection matrices...
    *
    * because of the tree hierarchy, we put the inherited "tree" transform in the view
    * and combined the projection and view into a  vpMat
    * in most cases this is invisible, but it helps with lighting since we need to keep the 
    * model transformations separate from the view space so that lighting calculations can happen 
    * BEFORE we transform the model into view (and projection) space.
    * 
    * */

    glm::mat4 mvp;
    unsigned int shaderID;

    if (!enabled) return;

    //assert(myMaterial != NULL);

    if (myMaterial == NULL)
        myMaterial = Material::materials["green"];

    glm::vec3 lightLoc = sg->light.position;
    glm::vec3 cameraLoc = sg->camera.position;

    shaderID =  myMaterial->use(sg->renderPass);

    glUniform1f(glGetUniformLocation(shaderID, "myTime"), elapsedTime += (float)deltaTime);

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "v"), 1, GL_FALSE, glm::value_ptr(treeMat));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "p"), 1, GL_FALSE, glm::value_ptr(vpMat));

    // because it is only once per model, another approach might be just to pre-multiply model, view and perspective 
    mvp = vpMat * treeMat * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glUniform3fv(glGetUniformLocation(shaderID, "cPos"), 1, glm::value_ptr(cameraLoc));
    glUniform3fv(glGetUniformLocation(shaderID, "lPos"), 1, glm::value_ptr(lightLoc));

    glm::mat4 lightViewProjection = sg->light.projection() * glm::lookAt(sg->light.position, sg->light.target, sg->light.up);
    
    glUniformMatrix4fv(glGetUniformLocation(shaderID,"lightSpaceMatrix"),1,GL_FALSE,glm::value_ptr(lightViewProjection));
    
    glBindVertexArray(VAO);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (indexCount < 0) 
        glDrawArraysInstanced(GL_TRIANGLES, 0, -indexCount, instances);
    else
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instances);
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
