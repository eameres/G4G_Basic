#pragma once

#include <vector>
#include "Material.h"

class SphereRenderer : public Renderer {
public:
    SphereRenderer(Material*, glm::mat4 m);
};

class Sphere
{
private:
    int numVertices;
    int numIndices;
    std::vector<int> indices;
    std::vector<float> verts;
    void init(int);
    float toRadians(float degrees);

public:
    Sphere();
    Sphere(int prec);
    int getNumVertices();
    int getNumIndices();
    std::vector<int> getIndices();
    std::vector<float> getVerts();
};