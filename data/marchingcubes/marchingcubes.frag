#version 150 core

uniform vec4 a_cubeColor;

in vec3 g_normal;

out vec4 fragColor;

const vec4 xColor = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 yColor = vec4(0.0, 1.0, 0.0, 1.0);
const vec4 zColor = vec4(0.0, 0.0, 1.0, 1.0);

void main()
{
    vec3 blend_weights = abs(g_normal);
    
    // Tighten up the blending zone:  
    blend_weights = (blend_weights - 0.2) * 7;   
    blend_weights = max(blend_weights, 0);      
    
    // Force weights to sum to 1.0 (very important!)  
    blend_weights /= vec3(blend_weights.x + blend_weights.y + blend_weights.z );
    
     // Finally, blend the results of the 3 planar projections.  
    vec4 blended_color = 
        xColor * vec4(blend_weights.x) +  
        yColor * vec4(blend_weights.y) +  
        zColor * vec4(blend_weights.z);
        
    fragColor = blended_color;
}
