#pragma once

#include "Material.h"
#include "SceneGraph.h"

struct SceneGraph;

class Renderer {
public:
    static std::vector<Renderer*> renderList;

protected:
    unsigned int VBO[8], VAO = 0, EBO = 0;
    int numVBOs = 1;
public :
    bool enabled = true;
    int indexCount;
    float elapsedTime = 0.0f;

    glm::mat4 modelMatrix;

    Material* myMaterial = NULL;

protected:
    void setupColorAttrib();

public:
    Renderer(){
        renderList.push_back(this);
    }
    ~Renderer() {
        glDeleteBuffers(numVBOs, VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
        
        std::vector<Renderer*>::const_iterator id;
            
        id = find(renderList.begin(), renderList.end(), this);

        if (id != renderList.end())
            renderList.erase(id);
    }

public: void setXForm(glm::mat4 mat)
{
    modelMatrix = mat;
}

public: void rotate(const float axis[], const float angle)
{
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(axis[0], axis[1], axis[2]));
}    

public: void translate(const float trans[])
{
    modelMatrix = glm::translate(modelMatrix, glm::vec3(trans[0], trans[1], trans[2]));
}
public: void setTranslate(glm::vec3 trans)
{
    modelMatrix = glm::mat4(glm::mat3(modelMatrix));
    modelMatrix = glm::translate(glm::mat4(1.0), trans) * modelMatrix;
}

public: void scale(const float scale[])
{
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale[0], scale[1], scale[2]));
}
public: 
    virtual void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg);

};

class ObjRenderer : public Renderer {
public:
    ObjRenderer(const char* filePath, Material*, glm::mat4 m);
};

class TorusRenderer : public Renderer {
public:
    TorusRenderer(Material*, glm::mat4 m);
};

class CubeRenderer : public Renderer {
public:
    CubeRenderer(Material*, glm::mat4 m);
};

class QuadRenderer : public Renderer {
public:
    QuadRenderer(Material*, glm::mat4 m);
};

class SkyboxRenderer : public Renderer {
    
public:
    SkyboxRenderer(Material*, glm::mat4 m);
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg);
};

class iCubeRenderer : public Renderer {
    
public:
    int maxParticles = 25000;
    int instances = 250;
    glm::mat4* iModelMatrices;
    glm::vec3* iModelColors;
    
public:
    iCubeRenderer(Material*, glm::mat4 m);
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg);
    
private:
    void setupIMatrices(void);
};

#include "SphereRenderer.h"

