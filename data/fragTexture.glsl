#version 410 core

out vec4 FragColor;

uniform float shine;

uniform sampler2D OurTexture;
uniform samplerCube EnvTexture;

in vec2 TexCoord;
in vec3 r;
in vec3 Normal;  
in vec3 FragPos;

uniform vec3 cPos;
uniform vec3 lPos;

vec3 lightColor = vec3(1.0,1.0,1.0); 

void main()
{
	vec4 envColor;

// diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

	
	envColor = texture(EnvTexture,r);
	vec4 texColor = texture(OurTexture, TexCoord);

	//texColor = vec4(.5,.5,.2,1.0);

    texColor = mix(texColor,vec4(diffuse,1),.25);
    FragColor = mix(envColor,texColor,1.0);
}