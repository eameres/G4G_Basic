#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in mat4 iMat;

uniform mat4 p,v,m;
out vec3 FragPos;
out vec3 Normal;

uniform vec4 ourColor;
uniform float myTime;

out vec4 varyingColor;

vec3 Axis = vec3(1,0,0);

vec3 myRotate(vec3 In)
{
	//return In;
    float s = sin(myTime*3);
    float c = cos(myTime*3);
    float one_minus_c = 1.0 - c;
	
	Axis = aColor;
    Axis = normalize(Axis);
    mat3 rot_mat = 
    mat3(vec3(one_minus_c * Axis.x * Axis.x + c, one_minus_c * Axis.x * Axis.y - Axis.z * s, one_minus_c * Axis.z * Axis.x + Axis.y * s),
        vec3(one_minus_c * Axis.x * Axis.y + Axis.z * s, one_minus_c * Axis.y * Axis.y + c, one_minus_c * Axis.y * Axis.z - Axis.x * s),
        vec3(one_minus_c * Axis.z * Axis.x - Axis.y * s, one_minus_c * Axis.y * Axis.z + Axis.x * s, one_minus_c * Axis.z * Axis.z + c)
    );
    return rot_mat * In;
}
void main()
{
	varyingColor = vec4(aColor,1.0);
	mat4 newM = m * iMat;
    FragPos = vec3(newM * vec4(myRotate(aPos), 1.0));
    Normal = mat3(transpose(inverse(newM))) * myRotate(aNormal);  
    
    gl_Position = p * v * newM* vec4(myRotate(aPos), 1.0);
}