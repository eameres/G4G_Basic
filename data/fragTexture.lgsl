#version 410 core

out vec4 FragColor;

uniform float shine;

uniform sampler2D OurTexture;
uniform samplerCube EnvTexture;

in vec2 TexCoord;
in vec3 r;

void main()
{
	vec4 envColor;
	
	envColor = texture(EnvTexture,r);
	vec4 texColor = texture(OurTexture, TexCoord);

	//texColor = vec4(.5,.5,.2,1.0);

	//FragColor = texture(OurTexture, TexCoord);
    FragColor = mix(texColor,envColor,shine);
}