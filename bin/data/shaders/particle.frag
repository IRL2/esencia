#version 150

uniform sampler2D particleTex;
uniform vec4 color;

out vec4 outputColor;

void main() {
    vec2 coord = gl_PointCoord;
    vec4 texColor = texture(particleTex, coord);
    
    // Calculate distance from center (0.5, 0.5) to current fragment
    vec2 center = vec2(0.5, 0.5);
    float distance = length(coord - center);
    
    // Create multiple glow layers for more realistic effect
    
    // Core particle (original texture)
    float coreAlpha = smoothstep(0.4, 0.32, distance * 2.0);
    vec4 coreColor = texColor * color * coreAlpha;
    
    // Inner glow (bright, tight)
    float innerGlow = exp(-distance * 8.0) * 1.5;
    innerGlow = smoothstep(0.0, 1.0, innerGlow);
    
    // Outer glow (softer, wider)
    float outerGlow = exp(-distance * 3.0) * 0.9;
    outerGlow = smoothstep(0.0, 1.0, outerGlow);
    
    // Soft edge fade
    float edgeFade = 1.0 - smoothstep(0.7, 1.0, distance * 2.0);
    
    // Combine glow layers
    float totalGlow = (innerGlow + outerGlow) * edgeFade;
    vec3 glowColor = color.rgb * totalGlow; // Use particle color for glow
    
    // Blend core particle with glow
    vec3 finalColor = coreColor.rgb + glowColor;
    // Apply the input alpha to the final result - this was the missing piece!
    float finalAlpha = max(coreColor.a, totalGlow * 0.8) * color.a;
    
    
    outputColor = vec4(finalColor, finalAlpha);
}