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

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D shadowMap;

float ShadowCalcPCF(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lPos - fs_in.FragPos);
    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.01);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if ((projCoords.x > 1) || (projCoords.y > 1) || (projCoords.z > 1))
		return 0;
	if ((projCoords.x < 0) || (projCoords.y < 0) || (projCoords.z < 0))
		return 0;
        
    return shadow;
}
float ShadowCalc(vec4 fragPosLightSpace)
{    
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	if ((projCoords.x > 1) || (projCoords.y > 1) || (projCoords.z > 1))
		return 0;
	if ((projCoords.x < 0) || (projCoords.y < 0) || (projCoords.z < 0))
		return 0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth -.01 > closestDepth  ? 1.0 : 0.0;

    return shadow;
}
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

    //texColor = mix(texColor,vec4(diffuse,1),.25);
   // FragColor = mix(envColor,texColor,1.0);
 float shadow = ShadowCalcPCF(fs_in.FragPosLightSpace);
	//shadow = 0.0; 
    //vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * varyingColor.xyz;
    
	float distance    = length(lPos - FragPos);
	float attenuation = 1.0 / (1.0 + 0.045 * distance + 
    		    0.0075 * (distance * distance));  

	diffuse *= attenuation;

    FragColor = vec4( (1.0 - shadow)*diffuse,1) * texColor;
}