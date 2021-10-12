//
//  drawImGui.hpp
//  g4g2
//
//  Created by Eric Ameres on 9/18/21.
//

#ifndef drawImGui_hpp
#define drawImGui_hpp

void drawIMGUI(Renderer *myRenderer, iCubeModel* cubeSystem,SceneGraph *sg,std::map<std::string,unsigned int> texMap,treeNode *[]);
void ShaderEditor(SceneGraph* sg);
void ListRenderers(SceneGraph* sg, treeNode* []);


#endif /* drawImGui_hpp */
