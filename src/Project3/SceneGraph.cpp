#include "SceneGraph.h"

void treeNode::traverse(glm::mat4 vMat, glm::mat4 projection, double deltaTime, RenderContext* RC) {

    vMat *= xform;

    for (Renderer* r : group)
    {
        r->render(vMat, projection, deltaTime, RC->light.position, RC->camera.position);
    }
    if (children.size() > 0) {
        for (treeNode* n : children) {
            n->traverse(vMat, projection, deltaTime, RC);
        }
    }
}