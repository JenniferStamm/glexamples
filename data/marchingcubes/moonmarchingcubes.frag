#version 150 core

uniform sampler2D base;
uniform sampler2D extra;

uniform vec4 a_cubeColor;
uniform bool useShadow;
uniform bool useOcclusion;
uniform bool useBaseTexture;
uniform bool useExtraTexture;

in vec3 v_normal;
in float v_occlusion;
in vec3 v_position;

out vec4 fragColor;

const vec3 lightDirection = vec3(1.0, 1.0, 1.0);

const float tex_scale = 0.4;

void main()
{
    vec3 blend_weights = abs(v_normal);
    
    // Tighten up the blending zone:  
    blend_weights = (blend_weights - 0.2) * 7;   
    blend_weights = max(blend_weights, 0);      
    
    // Force weights to sum to 1.0 (very important!)  
    blend_weights /= vec3(blend_weights.x + blend_weights.y + blend_weights.z );
    
    // Compute the UV coords for each of the 3 planar projections.  
    // tex_scale (default ~ 1.0) determines how big the textures appear.  
    vec2 coord1 = v_position.yz * tex_scale;  
    vec2 coord2 = v_position.zx * tex_scale;  
    vec2 coord3 = v_position.xy * tex_scale;
    
    vec3 xColor = texture(base, coord1).rgb;
    vec3 yColor = texture(base, coord2).rgb;
    vec3 zColor = texture(base, coord3).rgb;
    
     // Finally, blend the results of the 3 planar projections.  
    vec3 blended_color = 
        xColor * vec3(blend_weights.x) +  
        yColor * vec3(blend_weights.y) +  
        zColor * vec3(blend_weights.z);
        
    float shadow = dot(v_normal, lightDirection);
    
    fragColor = vec4(mix(v_normal, blended_color, float(useBaseTexture))* mix(1.0, smoothstep(-0.15, 1.0, v_occlusion), float(useOcclusion)) * mix(1.0, smoothstep(-0.2, 0.6, shadow), float(useShadow)), 1.0);
}
