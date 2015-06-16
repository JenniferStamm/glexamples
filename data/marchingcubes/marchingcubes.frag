#version 150 core

uniform sampler2D ground;

uniform vec4 a_cubeColor;

in vec3 v_normal;
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
    coord1.x = mod(coord1.x, 0.25);
    coord2.x = mod(coord2.x, 0.25);
    coord3.x = mod(coord3.x, 0.25);
    
    vec4 xColor = mix(texture(ground, coord1), texture(ground, coord1 + 0.75), 0.5);
    vec4 yColor = mix(texture(ground, coord2), texture(ground, coord2 + 0.75), 0.5);
    vec4 zColor = mix(texture(ground, coord2), texture(ground, coord2 + 0.75), 0.5);
    
     // Finally, blend the results of the 3 planar projections.  
    vec4 blended_color = 
        xColor * vec4(blend_weights.x) +  
        yColor * vec4(blend_weights.y) +  
        zColor * vec4(blend_weights.z);
        
    float shadow = dot(v_normal, lightDirection);
    
    fragColor = vec4(blended_color.xyz * smoothstep(-0.2, 0.6, shadow), 1.0);
    //fragColor = vec4(coord1, 0.0, 1.0);
}
