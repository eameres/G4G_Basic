#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective

out vec4 varyingColor;

void main()
{
	varyingColor = aColor;
	gl_Position = p*v*m*vec4(aPos, 1.0);
}