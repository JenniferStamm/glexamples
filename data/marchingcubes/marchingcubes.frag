#version 150 core

uniform sampler2D base;
uniform sampler2D striation;

uniform vec4 a_cubeColor;
uniform bool useShadow;
uniform bool useOcclusion;
uniform bool useBaseTexture;
uniform bool useStriationTexture;

in vec3 v_normal;
in float v_occlusion;
in vec3 v_position;

out vec4 fragColor;

const vec3 lightDirection = vec3(0.0, 1.0, 0.0);

const float tex_scale = 1.0;

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
    
    vec4 xColor = texture(base, coord1);
    vec4 yColor = texture(base, coord2);
    vec4 zColor = texture(base, coord3);
    
     // Finally, blend the results of the 3 planar projections.  
    vec4 blended_color = 
        xColor * vec4(blend_weights.x) +  
        yColor * vec4(blend_weights.y) +  
        zColor * vec4(blend_weights.z);
        
    // Add color from striation mainly dependent on height
    vec2 colorCoord = vec2(mod(v_position.y, 0.25) * 4 + 0.5, v_position.x) * 2.0;
    vec3 colorAddition = texture(striation, colorCoord).xyz;
    blended_color.xyz = mix(blended_color.xyz, colorAddition, 0.2 * float(useStriationTexture));
        
    float shadow = dot(v_normal, lightDirection);
    
    fragColor = vec4(mix(v_normal, blended_color.xyz, float(useBaseTexture))* mix(1.0, v_occlusion, float(useOcclusion)) * mix(1.0, smoothstep(-0.2, 0.6, shadow), float(useShadow)), 1.0);
}
