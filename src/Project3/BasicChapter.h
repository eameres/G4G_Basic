#pragma once

#include "Chapter0.h"


class BasicChapter : public Chapter0 {
public:
	BasicChapter() { name = "G4G:Chapter Template"; }
	void start();
	void update(double);
	void end();
	void callback(GLFWwindow* window, int width, int height);

	unsigned int depthMapFBO = 0;
	unsigned int offscreenFBO = 0;
	treeNode* nodes[5];

	std::map<std::string, unsigned int> texMap;

	const unsigned int SHADOW_WIDTH = 1024;
	const unsigned int SHADOW_HEIGHT = 1024;

	SceneGraph scene;
	SceneGraph* globalScene;

	unsigned int texture[4] = { 0,1,2,3 };

	CubeModel* lightCube = NULL;
	SkyboxModel* mySky;
	QuadModel* fQuad;
};

// settings
extern unsigned int scrn_width;
extern unsigned int scrn_height;