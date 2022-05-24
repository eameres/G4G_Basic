#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aCol;
layout (location = 3) in vec2 uv;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective

out vec4 vCol;
out vec2 TexCoord;

void main()
{
	TexCoord = uv;
	gl_Position = p*v*m*vec4(aPos, 1.0);
	vCol = vec4(aCol,1.0);
}