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

static const float vertices[288] = {
          0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f,1.0f,
          0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f,0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f,0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f,0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f,1.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f,1.0f,

         -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f,0.0f,
          0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f,0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f,1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f,1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f,1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f,0.0f,

         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f,0.0f,
         -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f,1.0f,
         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f,1.0f,
         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f,1.0f,
         -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f,0.0f,
         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f,0.0f,

          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f,1.0f,
          0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f,0.0f,
          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f,0.0f,
          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f,0.0f,
          0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f,1.0f,
          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f,1.0f,

         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f,1.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f,1.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f,0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f,0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f,0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f,1.0f,

          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f,0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f,1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f,1.0f,

         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f,1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f,0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f,0.0f
};

CubeModel::CubeModel(Material* material, glm::mat4 m)
{

    Renderer::name = "Cube";
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myMaterial = material;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(numVBOs = 2, VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // vertex buffer object, simple version, just coordinates

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // uv attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    setupColorAttrib(); // color is in its own VBO


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    indexCount = -36; // since we have no indices, we tell the render call to use raw triangles by setting indexCount to -numVertices

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}
static const float cubeVertexPositions[108] =
{
    // back face
    1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,  /**/ -1.0f, 1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

    // right face
    1.0f,  1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,  /**/  1.0f, 1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,

    // front face
    1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  /**/  1.0f, 1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,

    // left face
   -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  /**/ -1.0f, 1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,

   // bottom face
    1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,  /**/ -1.0f,-1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,

    // top face
    1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,  /**/ -1.0f, 1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f
};

static const float cubeVertexNormals[108] = {
     0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f, -1.0f,
     1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,  0.0f,
     0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,  1.0f,
    -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,  -1.0f,  0.0f,  0.0f,
     0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,   0.0f, -1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,  0.0f,
};
static const float cubeTexCoords[72] = {
    1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f
};

SkyboxModel::SkyboxModel(Material* material, glm::mat4 m)
{
    Renderer::name = "Skybox";
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myMaterial = material;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(numVBOs = 1, VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

void SkyboxModel::render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg)
{ // here's where the "actual drawing" gets done

    glm::mat4 mvp;

    glm::vec3 lightLoc = sg->light.position;
    glm::vec3 cameraLoc = sg->camera.position;

    if (!enabled) return;

    if (myMaterial == NULL)
        myMaterial = Material::materials["green"];

    unsigned int ID = myMaterial->use(sg->renderPass);

    myMaterial->myShader->setInt("skybox", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, myMaterial->textures[0]);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    mvp = pMat * vMat;

    glUniformMatrix4fv(glGetUniformLocation(ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(ID, "v"), 1, GL_FALSE, glm::value_ptr(glm::mat4(glm::mat3(vMat))));
    glUniformMatrix4fv(glGetUniformLocation(ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));

    glUniformMatrix4fv(glGetUniformLocation(ID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glUniform1f(glGetUniformLocation(ID, "myTime"), elapsedTime += (float)deltaTime);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void iCubeModel::setupIMatrices() {
    unsigned int amount = maxParticles;
    iModelMatrices = new glm::mat4[amount];
    iModelColors = new glm::vec3[amount];

    for (unsigned int i = 0; i < amount; i++)
    {

        iModelColors[i] = glm::vec3((float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3((rand() % 100) - 50, (rand() % 100) - 50, (rand() % 100) - 50));
        model = glm::rotate(model, (float)rand(), glm::vec3((rand() % 100) - 50, (rand() % 100) - 50, (rand() % 100) - 50));

        iModelMatrices[i] = model;
    }

    glBindVertexArray(VAO);

    // configure instanced array4
    // -------------------------
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, amount * (sizeof(glm::mat4) + sizeof(glm::vec3)), 0, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &iModelMatrices[0]);

    // set transformation matrices and color as an instance vertex attribute (with divisor 1)
    // -----------------------------------------------------------------------------------------------------------------------------------

    // set attribute pointers for matrix (4 times vec4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);

    {   // setup color per instance
        //glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferSubData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), amount * sizeof(glm::vec3), &iModelColors[0]);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(amount * sizeof(glm::mat4)));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);
    }
}

iCubeModel::iCubeModel(Material* material, glm::mat4 m)
{

    Renderer::name = "iCube";
    modelMatrix = m;
    myMaterial = material;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // position attribute
    glGenBuffers(numVBOs = 2, VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions)+ sizeof(cubeVertexNormals)+ sizeof(cubeTexCoords), 0, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeVertexPositions), cubeVertexPositions);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), sizeof(cubeVertexNormals), cubeVertexNormals);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)sizeof(cubeVertexPositions));
    glEnableVertexAttribArray(1);

    indexCount = -36; // since we have no indices, we tell the render call to use raw triangles by setting indexCount to -numVertices

    // texture attribute
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions)+ sizeof(cubeVertexNormals),sizeof(cubeTexCoords), cubeTexCoords);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(cubeVertexPositions) + sizeof(cubeVertexNormals)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //setupColorAttrib();
    setupIMatrices();

    glBindVertexArray(0);
    myMaterial->lastChange(0.0);

    instances = 250;
}
// havign a unique render routine is only necessary if you want to modify the VBO before calling render.
// perhaps we need a "preRender" method?

/*

void iCubeModel::render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg)
{ // here's where the "actual drawing" gets done

    unsigned int ID;
    glm::mat4 mvp;

    glm::vec3 lightLoc = sg->light.position;
    glm::vec3 cameraLoc = sg->camera.position;

    if (!enabled) return;

    if (myMaterial == NULL)
        myMaterial = Material::materials["green"];

    ID = myMaterial->use(sg->renderPass);

    glUniform3f(glGetUniformLocation(ID, "cPos"), -glm::vec3(vMat[3])[0], -glm::vec3(vMat[3])[1], -glm::vec3(vMat[3])[2]);
    glUniform3fv(glGetUniformLocation(ID, "lPos"), 1, glm::value_ptr(lightLoc));

    mvp = pMat * vMat * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(ID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(ID, "v"), 1, GL_FALSE, glm::value_ptr(vMat));
    glUniformMatrix4fv(glGetUniformLocation(ID, "p"), 1, GL_FALSE, glm::value_ptr(pMat));

    glUniformMatrix4fv(glGetUniformLocation(ID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glm::mat4 lightViewProjection = sg->light.projection() * glm::lookAt(sg->light.position, sg->light.target, sg->light.up);

    glUniformMatrix4fv(glGetUniformLocation(ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightViewProjection));
    glUniform1f(glGetUniformLocation(ID, "myTime"), elapsedTime += (float)deltaTime);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBindVertexArray(VAO);

    if (myMaterial->lastChange()+3.0 < elapsedTime) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        iModelColors = new glm::vec3[instances];

        for (unsigned int i = 0; i < instances; i++)
        {

            iModelColors[i] = glm::vec3((float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f);
        }

        glBufferSubData(GL_ARRAY_BUFFER, maxParticles * sizeof(glm::mat4), instances * sizeof(glm::vec3), &iModelColors[0]);

        myMaterial->lastChange(elapsedTime);
    }
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instances);
}
*/
