#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aCol;
layout (location = 3) in vec2 uv;

uniform mat4 m; // model
uniform mat4 v; // view
uniform mat4 p; // perspective
uniform float myTime;
uniform vec3 cPos;

out vec2 TexCoord;
out vec3 r;

uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} vs_out;

void main()
{
	vec3 aPos1 = aPos ;//+ (aNorm*abs(sin(myTime))*20);

	TexCoord = uv.xy;

    FragPos = vec3(v*m*vec4(aPos1, 1.0));
    Normal = normalize(mat3(transpose(inverse(v*m))) * normalize(aNorm)); 
	r = reflect((FragPos-cPos),Normal);

    vs_out.FragPos = FragPos;
    vs_out.Normal = Normal;
    vs_out.FragPosLightSpace = lightSpaceMatrix * v*m*vec4(aPos, 1.0);

    gl_Position = p*v*m*vec4(aPos1, 1.0);
    
}