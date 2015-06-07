#version 150 core

uniform sampler2D ground;

uniform vec4 a_cubeColor;

in vec3 g_normal;
in vec3 g_position;

out vec4 fragColor;

const float tex_scale = 1.0;

void main()
{
    vec3 blend_weights = abs(g_normal);
    
    // Tighten up the blending zone:  
    blend_weights = (blend_weights - 0.2) * 7;   
    blend_weights = max(blend_weights, 0);      
    
    // Force weights to sum to 1.0 (very important!)  
    blend_weights /= vec3(blend_weights.x + blend_weights.y + blend_weights.z );
    
    // Compute the UV coords for each of the 3 planar projections.  
    // tex_scale (default ~ 1.0) determines how big the textures appear.  
    vec2 coord1 = g_position.yz * tex_scale;  
    vec2 coord2 = g_position.zx * tex_scale;  
    vec2 coord3 = g_position.xy * tex_scale;
    coord1.x = mod(coord1.x, 0.25);
    coord2.x = mod(coord2.x, 0.25);
    coord3.x = mod(coord3.x, 0.25);
    
    vec4 xColor = texture(ground, coord1);
    vec4 yColor = texture(ground, coord2);
    vec4 zColor = texture(ground, coord3);
    
     // Finally, blend the results of the 3 planar projections.  
    vec4 blended_color = 
        xColor * vec4(blend_weights.x) +  
        yColor * vec4(blend_weights.y) +  
        zColor * vec4(blend_weights.z);
        
    fragColor = blended_color;
    //fragColor = vec4(coord1, 0.0, 1.0);
}
