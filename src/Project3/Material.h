#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <map>
#include "shader_s.h"


// materials currently only include basic diffuse and specular properties
// there is room for more textures, but needs to be exanded
// materials may also need to include multiple shaders for differnt render passes
class Material {
public:
    Shader* myShader;
    std::string name;
    GLint textures[8] = { 0,0,0,0,0,0,0,0 };
    float shine = 0.1f;
    bool shadow = false;

    glm::vec4 color;

    static std::map<std::string,Material*> materials;
    double _lastChange;

    void lastChange(double _lc) { _lastChange = _lc; }
    double lastChange() { return _lastChange; }

public:
    Material(Shader* _shader, std::string _name, GLint _texture, glm::vec4 _color) {
        assert(_shader != NULL);
        myShader = _shader;
        textures[0] = _texture;
        color = _color;
        name = _name;
        materials[name] = this;
    }
    Material(Shader* _shader, std::string _name, GLint _texture, GLint _envTexture) {
        assert(_shader != NULL);
        myShader = _shader;
        textures[0] = _texture;
        textures[1] = _envTexture;
        color = glm::vec4(1.0, 1.0, 1.0, 1.0);
        shadow = false;
        name = _name;
        materials[name] = this;
    }
    Material(Shader* _shader, std::string _name, GLint _texture, GLint depthMap, bool _shadow) {
        assert(_shader != NULL);
        myShader = _shader;
        textures[0] = _texture;
        textures[1] = depthMap;
        color = glm::vec4(1.0, 1.0, 1.0, 1.0);
        shadow = _shadow;
        name = _name;
        materials[name] = this;
    }
    ~Material() {
        materials.erase(name);
    }

    unsigned int use(enum SceneGraph::rp enc) {

        if (enc == SceneGraph::SHADOW) { // try to use a simplified shader if we're in the shadow pass
            if ((myShader != Shader::shaders["SkyBox"]) && (myShader != Shader::shaders["Particle"])) {
                Shader::shaders["Depth"]->use();
                return Shader::shaders["Depth"]->ID;
            }
        }
        assert(myShader != NULL);

        myShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        myShader->setInt("OurTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        myShader->setInt("shadowMap", 1);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textures[2]);
        myShader->setInt("EnvTexture", 2);

        glUniform1f(glGetUniformLocation(myShader->ID, "shine"), shine);

        glUniform4fv(glGetUniformLocation(myShader->ID, "ourColor"), 1, glm::value_ptr(color));

        return myShader->ID;
    }
};
