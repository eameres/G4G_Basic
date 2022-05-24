#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective

out vec4 varyingColor;

mat4 front = mat4(
0.5,0,0,0,
0,0.5,0,0,
0,0,0.5,0,
0,0,0.27,1);

mat4 back = mat4(
0.5,0,0,0,
-0,-0.5,0,-0,
0,0,-0.5,0,
0,0,-0.27,1);

mat4 right = mat4(
0,0,-0.5,0,
0,0.5,0,0,
0.5,0,0,0,
0.27,0,0,1);

mat4 left= mat4(
0,0,0.5,0,
0,0.5,0,0,
-0.5,0,0,0,
-0.27,0,0,1);

mat4 top = mat4(
0.5,0,0,0,
-0,0,-0.5,-0,
0,0.5,0,0,
0,0.27,0,1);

mat4 bottom = mat4(
0.5,0,0,0,
0,0,0.5,0,
-0,-0.5,0,-0,
0,-0.27,0,1);

void main()
{
	mat4 xf = front;

	if (gl_InstanceID == 1) xf = front;
	else if (gl_InstanceID == 2) xf = back;
	else if (gl_InstanceID == 3) xf = right;
	else if (gl_InstanceID == 4) xf = left;
	else if (gl_InstanceID == 5) xf = top;
	else if (gl_InstanceID == 6) xf = bottom;

	varyingColor = aColor;
	gl_Position = p*v*m*xf*vec4(aPos, 1.0);
}