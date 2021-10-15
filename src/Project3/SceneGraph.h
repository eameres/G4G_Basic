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

struct Orthographic {
    float clipNear, clipFar;
    float clipRight, clipLeft;
    float clipBottom, clipTop;
};

struct Perspective {
    float clipNear, clipFar;
    float aspect, fov;
    float width, height;
};

struct emitterCollector { // a light or camera common attributes
private:
    glm::mat4 _projection;
    enum { ORTHO, PERSP } pType;
    union {
        Perspective perspective;
        Orthographic orthographic;
    };

public:
    enum { LIGHT, CAMERA } ecType;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    glm::mat4 projection() { return _projection; }

    float getFOV() { return perspective.fov; }
    void setFOV(float fov) { setPerspective(fov, perspective.aspect, perspective.clipNear, perspective.clipFar); }

    float getAspect() { return perspective.aspect; }
    void setAspect(float aspect) { setPerspective(perspective.fov, perspective.aspect = aspect, perspective.clipNear, perspective.clipFar); }

    void getClipNearFar(float* near, float *far) {*near = perspective.clipNear, *far = perspective.clipFar; }
    void setClipNearFar(float near, float far) {
        if (pType == PERSP)
            setPerspective(perspective.fov, perspective.aspect, perspective.clipNear = near, perspective.clipFar = far);
        else if (pType == ORTHO)
            setOrtho(orthographic.clipLeft, orthographic.clipRight, orthographic.clipTop, orthographic.clipBottom, orthographic.clipNear = near, orthographic.clipFar = far);
    }

    void setPerspective(float fov, float aspect, float clipNear, float clipFar) {
        pType = PERSP;
        _projection = glm::perspective(perspective.fov = fov, perspective.aspect = aspect, perspective.clipNear = clipNear, perspective.clipFar = clipFar);
    }

    void setOrtho(float clipLeft, float clipRight, float clipBottom,float clipTop , float clipNear, float clipFar) {
        pType = ORTHO;
        _projection = glm::ortho(orthographic.clipLeft = clipLeft, orthographic.clipRight = clipRight, orthographic.clipTop = clipTop, orthographic.clipBottom = clipBottom, orthographic.clipNear = clipNear, orthographic.clipFar = clipFar);
    }
};

struct SceneGraph;
class Renderer;

struct treeNode {

private:
    std::vector<treeNode*> children;
    std::vector<Renderer*> group;
    glm::mat4 xform;
    treeNode* parent;

    glm::vec3 _translate;
    glm::vec3 _scale;
    glm::vec3 _rotationEuler;

public:
    bool enabled;

public:
    treeNode(glm::mat4 xf, treeNode* p) {
        xform = xf;
        parent = p;
        enabled = true;
    }
    void setXform(glm::mat4 xf) { xform = xf; }

    void addRenderer(Renderer* r) { group.push_back(r); }
    treeNode* addChild(glm::mat4 xf) { children.push_back(new treeNode(xf, this)); return children.back(); }
    treeNode* getParent() { return this->parent; }
    std::vector<treeNode*> *getChildren() { return &children; };

    void traverse(glm::mat4 vMat, glm::mat4 projection, double deltaTime, SceneGraph* sg);
    void purgeRenderer(Renderer *x);
};

class SceneGraph {
public:
    emitterCollector camera;
    emitterCollector light;

    enum rp { SHADOW, REGULAR } renderPass = REGULAR;
    
    std::vector<Renderer*> rendererList;

    treeNode* tree;
    treeNode* currNode;
    double time = 0.0;

public:

    void addRenderer(Renderer* r) { 
        currNode->addRenderer(r); 
        rendererList.push_back(r);
    }

    void purgeRenderer(Renderer* x);

    treeNode* getCurrentNode() {
        return currNode;
    }    

    treeNode* getRoot() {
        return tree;
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
        
        camera.ecType = emitterCollector::CAMERA;
        light.ecType = emitterCollector::LIGHT;
    
        camera.up = glm::vec3(0.0, 1.0, 0.0);
        light.up = glm::vec3(0.0, 1.0, 0.0);
        currNode = tree = new treeNode(glm::mat4(1), (treeNode*)NULL);
    }

    void renderFrom(emitterCollector ec, double deltaTime) {
        
        if (ec.ecType == emitterCollector::LIGHT){
            renderPass = SHADOW;
        }else
            renderPass = REGULAR;
        
        // we use a combined projection * view matrix 
        // and a "treeview" matrix to hold the tree transform which gets combined with the models later
        glm::mat4 vpMat = ec.projection() * glm::lookAt(ec.position, ec.target, ec.up);
        glm::mat4 tvMat = glm::mat4(1.0);

        tree->traverse(tvMat, vpMat, deltaTime, this);
    }
};

#endif
