#include "SceneGraph.h"

void treeNode::traverse(glm::mat4 vMat, glm::mat4 projection, double deltaTime, SceneGraph *sg) {

    vMat *= xform;

    for (Renderer* r : group)
    {
        r->render(vMat, projection, deltaTime, sg);
    }
    if (children.size() > 0) {
        for (treeNode* n : children) {
            n->traverse(vMat, projection, deltaTime, sg);
        }
    }
}