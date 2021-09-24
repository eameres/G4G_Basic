#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos; 
in vec4 varyingColor;

vec3 lightColor = vec3(0.9,0.9,1.0); 

uniform vec3 cPos;
uniform vec3 lPos;

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

    	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
		spec = pow(max(dot(norm, halfwayDir), 0.0), 64);

		specular = specularStrength * spec * lightColor; 
    }
    vec3 result = (ambient + diffuse + specular) * varyingColor.xyz;
    FragColor = vec4(result,1.0);
} 