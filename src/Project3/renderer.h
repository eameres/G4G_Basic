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
    Material* depthMaterial = NULL;

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

class ObjModel : public Renderer {
public:
    ObjModel(const char* filePath, Material*, glm::mat4 m);
};

class TorusModel : public Renderer {
public:
    TorusModel(Material*, glm::mat4 m);
};

class CubeModel : public Renderer {
public:
    CubeModel(Material*, glm::mat4 m);
};

class QuadModel : public Renderer {
public:
    QuadModel(Material*, glm::mat4 m);
};

class SkyboxModel : public Renderer {
    
public:
    SkyboxModel(Material*, glm::mat4 m);
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg);
};

class iCubeModel : public Renderer {
    
public:
    int maxParticles = 25000;
    int instances = 250;
    glm::mat4* iModelMatrices;
    glm::vec3* iModelColors;
    
public:
    iCubeModel(Material*, glm::mat4 m);
    void render(glm::mat4 vMat, glm::mat4 pMat, double deltaTime, SceneGraph *sg);
    
private:
    void setupIMatrices(void);
};

#include "SphereModel.h"

