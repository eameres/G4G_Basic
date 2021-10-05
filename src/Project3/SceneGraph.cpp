#include "renderer.h"

void treeNode::traverse(glm::mat4 treeModelMat, glm::mat4 viewProjection, double deltaTime, SceneGraph *sg) {

    // traverse a treeNode, notice how this is called recursively, so we will travel down and back up the tree
    treeModelMat *= xform;

    if (enabled) { // if the node is not enable, no traversal occurs (all children are disabled by association)

        // first step, render any models pointed to by this node, note that they do have their own model Matrices, so no need to have one node per model
        for (Renderer* r : group)
        {
            r->render(treeModelMat, viewProjection, deltaTime, sg);
        }
        // if there are any children nodes, call them, passing down the concatenated "treeView" matrix
        if (children.size() > 0) {
            for (treeNode* n : children) {
                n->traverse(treeModelMat, viewProjection, deltaTime, sg);
            }
        }
    }
}