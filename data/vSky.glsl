#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 p;
uniform mat4 v;

void main()
{
    TexCoords = aPos;
    gl_Position = p * v * vec4(aPos, 1.0);
}  