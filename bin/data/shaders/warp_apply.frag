#version 150

uniform sampler2D sourceTexture;        // Video frame to warp
uniform sampler2D displacementTexture;  // Displacement from pass 1
uniform vec4 videoColor;
uniform float warpBrightPassThreshold;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    // Sample displacement
    vec2 displacement = texture(displacementTexture, vTexCoord).rg;
    
    // Apply displacement
    vec2 warpedCoord = vTexCoord + displacement;
    
    warpedCoord = fract(warpedCoord + 1.0);
    
    vec4 warpedColor = texture(sourceTexture, warpedCoord);
    
    // Apply brightness threshold
    if (warpBrightPassThreshold > 0.0) {
        vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);
        float luminance = dot(warpedColor.rgb, luminanceVector);
        luminance = max(0.0, luminance - warpBrightPassThreshold);
        warpedColor.rgb *= (luminance + warpBrightPassThreshold);
    }
    
    fragColor = warpedColor * videoColor;
}