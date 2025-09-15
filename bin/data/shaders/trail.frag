/* Fragment shader for particle trails */
#version 330 core

in float pointSize; // Receive particle size from vertex shader
out vec4 fragColor;

uniform sampler2D particleTexture; // Use the particle texture, not trail texture
uniform vec4 color;
uniform float fadeFactor;

void main() {
    // Use the point coordinates directly - gl_PointSize already handles the scaling
    vec2 coord = gl_PointCoord;
    
    // Sample the particle texture
    vec4 texColor = texture(particleTexture, coord);
    
    // Apply the color and fade factor
    fragColor = texColor * color * fadeFactor;
}