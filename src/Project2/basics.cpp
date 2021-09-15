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

int myTexture() 
{
	memset(imageBuff, 0, sizeof(imageBuff));

	for (int i = 0; i < dimx; i++)
		for (int j = 0; j < dimy; j++)
		{
			if (((i / 16) % 2) == 0)
			{
				if (((j / 16) % 2) == 0)
					imageBuff[i][j][0] = imageBuff[i][j][1] = imageBuff[i][j][2] = 255;
			}
			else {
				if (((j / 16) % 2) == 1)
					imageBuff[i][j][0] = imageBuff[i][j][1] = imageBuff[i][j][2] = 255;
			}
		}

	return 0;
}