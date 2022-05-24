#pragma once

void setupTextures(unsigned int textures[]);
void deleteTextures(unsigned int textures[]);

unsigned int loadTexture(const char* fPath);

class Texture {
public:
	static std::map<std::string, unsigned int> texMap;
};