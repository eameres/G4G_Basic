#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos; 
in vec4 varyingColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

vec3 lightColor = vec3(0.9,0.9,1.0); 

uniform vec3 cPos;
uniform vec3 lPos;

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
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
	vec3 specular = vec3(0,0,0);
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

	if (diff > 0.0) 
	{
    	// specular
    	float specularStrength = diff;
    	vec3 viewDir = normalize(cPos - FragPos);
    	vec3 reflectDir = reflect(-lightDir, norm);  

  	vec3 halfwayDir = normalize(lightDir + viewDir);

    	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); //Phong
		spec = pow(max(dot(norm, halfwayDir), 0.0), 64); // Blinn-Phong
		specular = specularStrength * spec * lightColor; 
    }
	// calculate shadow
    float shadow = ShadowCalcPCF(fs_in.FragPosLightSpace);
	//shadow = 0.0; 
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * varyingColor.xyz;
    
    FragColor = vec4(result,1.0);
} 