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

void main()
{
	vec3 aPos1 = aPos ;//+ (aNorm*abs(sin(myTime))*20);

	TexCoord = uv.xy;

    vec3 FragPos = vec3(v*m*vec4(aPos1, 1.0));
    vec3 Normal = normalize(mat3(transpose(inverse(v*m))) * normalize(aNorm)); 
	r = reflect((FragPos-cPos),Normal);

    gl_Position = p*v*m*vec4(aPos1, 1.0);
    
}