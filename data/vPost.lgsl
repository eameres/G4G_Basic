#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aCol;
layout (location = 3) in vec2 uv;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective
uniform float myTime;

out vec2 TexCoord;

void main()
{
	TexCoord = uv.xy;
	gl_Position = p*v*m*vec4(aPos, 1.0);
    
}