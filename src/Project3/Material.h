#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "shader_s.h"

class Material {
public:
    Shader* myShader;
    GLint textures[8] = { 0,0,0,0,0,0,0,0 };
    float shine = 0.1f;
    bool shadow = false;

    glm::vec4 color;

    static std::vector<Material*> materialList;

public:
    Material(Shader* _shader, GLint _texture, glm::vec4 _color) {
        myShader = _shader;
        textures[0] = _texture;
        color = _color;

        materialList.push_back(this);
    }
    Material(Shader* _shader, GLint _texture, GLint _envTexture) {
        myShader = _shader;
        textures[0] = _texture;
        textures[1] = _envTexture;
        color = glm::vec4(1.0, 1.0, 1.0, 1.0);
        shadow = false;
        materialList.push_back(this);
    }
    Material(Shader* _shader, GLint _texture, GLint depthMap, bool _shadow) {
        myShader = _shader;
        textures[0] = _texture;
        textures[1] = depthMap;
        color = glm::vec4(1.0, 1.0, 1.0, 1.0);
        shadow = _shadow;
        materialList.push_back(this);
    }
    ~Material() {
        std::vector<Material*>::const_iterator id;

        id = find(materialList.begin(), materialList.end(), this);

        if (id != materialList.end())
            materialList.erase(id);
    }

    void use() {

        myShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        myShader->setInt("OurTexture", 0);

        glActiveTexture(GL_TEXTURE1);

        if (shadow)
            glBindTexture(GL_TEXTURE_2D, textures[1]);
        else
            glBindTexture(GL_TEXTURE_CUBE_MAP, textures[1]);

        myShader->setInt("EnvTexture", 1);
        myShader->setInt("shadowMap", 1);

        glUniform4fv(glGetUniformLocation(myShader->ID, "ourColor"), 1, glm::value_ptr(color));
    }
};