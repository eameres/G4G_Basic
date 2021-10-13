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

void treeNode::purgeRenderer(Renderer *x) {

    // first step, remove model's occurances pointed to by this node
    // do this in reverse to avoid problems in vector changing
    for (int i = group.size() - 1; i >= 0; i--)
    {
        if (group[i] == x)
        group.erase(group.begin()+i);
    }
    // if there are any children nodes, call them, passing down the concatenated "treeView" matrix
    if (children.size() > 0) {
        for (treeNode* n : children) {
            n->purgeRenderer(x);
        }
    }
}

void SceneGraph::purgeRenderer(Renderer* x) {
    
    // purge from the tree
    tree->purgeRenderer(x);

    // purge from the scene's list
    for (int i = rendererList.size() - 1; i >= 0; i--)
    {
        if (rendererList[i] == x)
            rendererList.erase(rendererList.begin() + i);
    }

    // purge from the master list
    for (int i = Renderer::renderList.size() - 1; i >= 0; i--)
    {
        if (Renderer::renderList[i] == x) {
            Renderer::renderList.erase(Renderer::renderList.begin() + i);
        }
    }
    delete (x);
}