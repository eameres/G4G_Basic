#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective
uniform mat4 mvp; // combined

out vec4 vCol;

void main()
{
	gl_Position = mvp*vec4(aPos, 1.0);
	vCol = vec4(aCol,1.0);
}