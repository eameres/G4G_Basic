#version 410 core

out vec4 FragColor;

uniform sampler2D OurTexture;
uniform float myTime;

in vec2 TexCoord;

vec4 invert()
{
	return vec4(vec3(1.0 - texture(OurTexture, TexCoord)), 1.0);
}

const float offset = 1.0 / 500.0;  

vec4 postProc ()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    float kernel[9] = float[](
        1.0/9.0, 1.0/9.0,1.0/9.0,
        1.0/9.0,3.0/9.0,1.0/9.0,
        1.0/9.0,1.0/9.0,1.0/9.0
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(OurTexture, TexCoord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    //vec3 invCol = vec3(1.0 - col);
    vec3 invCol = texture(OurTexture, TexCoord.st).rgb;
    
    float dist = distance(TexCoord.xy, vec2(0.5, 0.5));
    col.rgb *= smoothstep(0.9, .1 * 0.799, dist * (.9));
    
    return vec4(mix(col,invCol,(sin(myTime)+1)/2),1.0);
}

void main(){
    //FragColor = postProc();
    //FragColor = invert();
    FragColor = texture(OurTexture, TexCoord.st);
}