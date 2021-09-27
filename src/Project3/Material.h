#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "shader_s.h"

class Material {
public:
    Shader* myShader;
    GLint texture;
    GLint envTexture = 0;
    float shine = 0.1f;
    bool shadow = false;

    glm::vec4 color;

    static std::vector<Material*> materialList;

public:
    Material(Shader* _shader, GLint _texture, glm::vec4 _color) {
        myShader = _shader;
        texture = _texture;
        color = _color;

        materialList.push_back(this);
    }
    Material(Shader* _shader, GLint _texture, GLint _envTexture) {
        myShader = _shader;
        texture = _texture;
        envTexture = _envTexture;
        color = glm::vec4(1.0, 1.0, 1.0, 1.0);
        shadow = false;
        materialList.push_back(this);
    }
    Material(Shader* _shader, GLint _texture, GLint _envTexture, bool _shadow) {
        myShader = _shader;
        texture = _texture;
        envTexture = _envTexture;
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
        glBindTexture(GL_TEXTURE_2D, texture);
        myShader->setInt("OurTexture", 0);

        glActiveTexture(GL_TEXTURE1);

        if (shadow)
            glBindTexture(GL_TEXTURE_2D, envTexture);
        else
            glBindTexture(GL_TEXTURE_CUBE_MAP, envTexture);

        myShader->setInt("EnvTexture", 1);

        myShader->setInt("shadowMap", 1);

        glUniform4fv(glGetUniformLocation(myShader->ID, "ourColor"), 1, glm::value_ptr(color));
    }
};