
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "shader_s.h"
#include "ImportedModel.h"

#include "camera.h"
#include "renderer.h"

#include "textures.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// image buffer used by raster drawing basics.cpp
extern unsigned char imageBuff[512][512][4];

int myTexture();
int RayTracer();

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int tNum;
    glGenTextures(1, &tNum);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tNum);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return tNum;
}

void setupTexture(unsigned int tNum, const void* buff, int x, int y, unsigned int fmt)
{
    glBindTexture(GL_TEXTURE_2D, tNum); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, x, y, 0, fmt, GL_UNSIGNED_BYTE, buff);
    glGenerateMipmap(GL_TEXTURE_2D);

}
void deleteTextures(unsigned int texture[])
{
    glDeleteTextures(3, texture);
}
void setupTextures(unsigned int texture[])
{
    // create textures 
    // -------------------------
    glGenTextures(2, texture);

    myTexture();
    setupTexture(texture[0], (const void*)imageBuff, 512, 512, GL_RGBA);
    // texture is a buffer we will be generating for pixel experiments

    RayTracer();
    setupTexture(texture[1], (const void*)imageBuff, 512, 512, GL_RGBA);

    // load textures
// -------------
    std::vector<std::string> faces
    {
        "data/cubemap/xp.jpg",
        "data/cubemap/xn.jpg",
        "data/cubemap/yp.jpg",
        "data/cubemap/yn.jpg",
        "data/cubemap/zp.jpg",
        "data/cubemap/zn.jpg",
    };
    texture[2] = loadCubemap(faces);
}

unsigned int loadTexture(const char* fPath)
{
    unsigned int oneOff;
    // create textures 
    // -------------------------
    glGenTextures(1, &oneOff);

    // load image, create texture and generate mipmaps
    int width = 0, height = 0, nrChannels = 0;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(fPath, &width, &height, &nrChannels, 0);

    setupTexture(oneOff, (const void*)data, width, height, nrChannels == 4 ? GL_RGBA : GL_RGB);

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
    return oneOff;
}

std::map<std::string, unsigned int> Texture::texMap;
