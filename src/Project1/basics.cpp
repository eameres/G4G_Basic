#include <fstream>
#include <cstdio>
#include <iostream>
#include <list>

struct myEvent {
	float time;
	int thing;
};

using namespace std;

constexpr auto dimx = 512u, dimy = 512u;

unsigned char imageBuff[dimx][dimy][3];

#define RED 0
#define GREEN 1
#define BLUE 2

int myTexture() 
{
	memset(imageBuff, 0, sizeof(imageBuff));
	
	for (int i = 0; i < dimx; i++)
		for (int j = 0; j < dimy; j++)
		{
			if (((i / 16) % 2) == 0)
			{
				if (((j / 16) % 2) == 0) {
					imageBuff[i][j][RED] = 0;
					imageBuff[i][j][GREEN] = 0;
					imageBuff[i][j][BLUE] = 255;
				}
			}
			else {
				if (((j / 16) % 2) == 1) {
					imageBuff[i][j][RED] = 0;
					imageBuff[i][j][GREEN] = 0;
					imageBuff[i][j][BLUE] = 255;
				}
			}
		}

	return 0;
}