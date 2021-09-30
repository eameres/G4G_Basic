#pragma once

#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"

struct emitterCollector { // a light or camera common attributes
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    glm::mat4 projection;
};

struct SceneGraph;
class Renderer;

struct treeNode {

private:
    std::vector<treeNode*> children;
    std::vector<Renderer*> group;
    glm::mat4 xform;
    treeNode* parent;

public:
    treeNode(glm::mat4 xf, treeNode* p) {
        xform = xf;
        parent = p;
    }
    void setXform(glm::mat4 xf) { xform = xf; }

    void addRenderer(Renderer* r) { group.push_back(r); }
    treeNode* addChild(glm::mat4 xf) { children.push_back(new treeNode(xf, this)); return children.back(); }
    treeNode* getParent() { return this->parent; }

    void traverse(glm::mat4 vMat, glm::mat4 projection, double deltaTime, SceneGraph * sg);
};

struct SceneGraph {

    emitterCollector camera;
    emitterCollector light;

    std::vector<Renderer*> rendererList;

    treeNode* tree;
    treeNode* currNode;
    double time;

public:
    void addRenderer(Renderer* r) { 
        currNode->addRenderer(r); 
        rendererList.push_back(r);
    }

    treeNode* addChild(glm::mat4 xf) {
        currNode = currNode->addChild(xf);
        return currNode;
    }

    treeNode* getParent() {
        currNode = currNode->getParent();
        return currNode;
    }

    void setXform(glm::mat4 xf) {
        currNode->setXform(xf);
    }

    SceneGraph() {
        camera.up = glm::vec3(0.0, 1.0, 0.0);
        light.up = glm::vec3(0.0, 1.0, 0.0);
        currNode = tree = new treeNode(glm::mat4(1), (treeNode*)NULL);
    }

    void renderFrom(emitterCollector ec, double deltaTime) {

        glm::mat4 pMat = ec.projection * glm::lookAt(ec.position, ec.target, ec.up);
        glm::mat4 mmMat = glm::mat4(1.0);

        tree->traverse(mmMat, pMat, deltaTime, this);
    }
};

#endif