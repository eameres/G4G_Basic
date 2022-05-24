#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aCol;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective
uniform vec4 ourColor;
uniform float myTime;

uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec4 varyingColor;

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} vs_out;

void main()
{

    FragPos = vec3(v*m * vec4( aPos, 1.0)  );
    Normal = transpose(inverse(mat3(v*m))) * aNormal;  

    vs_out.FragPos = FragPos;
    vs_out.Normal = Normal;
    vs_out.FragPosLightSpace = lightSpaceMatrix * v*m*vec4(aPos, 1.0);
    
    varyingColor = mix(vec4(aCol,1.0),ourColor,.5);

    
    gl_Position = p * v * m * vec4(aPos, 1.0);
}

