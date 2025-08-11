#version 150

uniform sampler2D currentFrame;
uniform sampler2D previousFrame;
uniform vec4 videoColor;
uniform vec4 videoRect;
uniform float time;
uniform vec2 resolution;
uniform float feedbackAmount;

// Warp effect parameters
uniform float warpVariance;
uniform float warpPropagation;
uniform float warpPropagationPersistence;
uniform float warpSpreadX;
uniform float warpSpreadY;
uniform float warpDetail;
uniform float warpBrightPassThreshold;
uniform bool useWarpEffect; // Switch between simple feedback and warp

in vec2 vTexCoord;
out vec4 fragColor;

// Function to calculate warp displacement based on current and previous frames
vec2 calculateWarpDisplacement(vec2 texCoord) {
    vec2 scaledCoord = texCoord * warpDetail;
    vec2 texelSize = 1.0 / textureSize(currentFrame, 0);
    
    vec2 spreadX = vec2(warpSpreadX * texelSize.x, 0.0);
    vec2 spreadY = vec2(0.0, warpSpreadY * texelSize.y);
    
    // Sample neighboring positions
    vec2 left = scaledCoord - spreadX;
    vec2 right = scaledCoord + spreadX;
    vec2 top = scaledCoord - spreadY;
    vec2 bottom = scaledCoord + spreadY;
    
    // Get current frame gradients (using red channel for luminance-based warping)
    float current_Center = texture(currentFrame, scaledCoord).r;
    float current_Right = texture(currentFrame, right).r;
    float current_Bottom = texture(currentFrame, bottom).r;
    
    // Calculate new displacement based on gradients
    vec2 newDisplacement = vec2(
        (current_Center - current_Right) * warpVariance,
        (current_Center - current_Bottom) * warpVariance
    );
    
    // Get previous frame's displacement (stored in previous frame's RG channels)
    // We'll encode displacement in the alpha and unused channels, but for now use a simple approach
    vec2 previousDisplacement = vec2(0.0); // This would come from a dedicated displacement buffer
    
    // For this implementation, we'll create displacement from the previous frame gradients
    float prev_Center = texture(previousFrame, scaledCoord).r;
    float prev_Left = texture(previousFrame, left).r;
    float prev_Right = texture(previousFrame, right).r;
    float prev_Top = texture(previousFrame, top).r;
    float prev_Bottom = texture(previousFrame, bottom).r;
    
    // Simulate propagation by using neighboring gradients from previous frame
    vec2 propagatedDisplacement = vec2(
        (prev_Center * warpPropagationPersistence) + 
        ((prev_Left + prev_Right + prev_Top + prev_Bottom) * warpPropagation * 0.25),
        (prev_Center * warpPropagationPersistence) + 
        ((prev_Left + prev_Right + prev_Top + prev_Bottom) * warpPropagation * 0.25)
    );
    
    // Combine new displacement with propagated displacement
    vec2 finalDisplacement = newDisplacement + propagatedDisplacement * 0.1;
    
    // Clamp to prevent extreme distortions
    finalDisplacement = clamp(finalDisplacement, vec2(-0.1), vec2(0.1));
    
    return finalDisplacement;
}

// Function to sample with warp displacement
vec4 sampleWarped(sampler2D tex, vec2 texCoord) {
    vec2 displacement = calculateWarpDisplacement(texCoord);
    vec2 warpedCoord = texCoord + displacement;
    
    // Ensure we stay within texture bounds
    warpedCoord = clamp(warpedCoord, vec2(0.0), vec2(1.0));
    
    return texture(tex, warpedCoord);
}

// Simple feedback function without warp
vec4 sampleSimpleFeedback(vec2 texCoord) {
    vec4 currentColor = texture(currentFrame, texCoord);
    vec4 previousColor = texture(previousFrame, texCoord);
    
    // Simple blend between current and previous frames
    return mix(currentColor, previousColor, feedbackAmount * 0.1);
}

void main() {
    vec4 finalColor;
    
    if (useWarpEffect) {
        // === WARP EFFECT RENDERING PATH ===
        // Sample current frame with warp effect
        vec4 warpedCurrent = sampleWarped(currentFrame, vTexCoord);
        
        // Optional: Add some feedback with the previous frame for temporal coherence
        vec4 previousColor = texture(previousFrame, vTexCoord);
        
        // Blend warped current with previous frame for smoother animation
        finalColor = mix(warpedCurrent, previousColor, feedbackAmount * 0.1);
        
        // Apply brightness threshold if desired
        if (warpBrightPassThreshold > 0.0) {
            vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);
            float luminance = dot(finalColor.rgb, luminanceVector);
            luminance = max(0.0, luminance - warpBrightPassThreshold);
            finalColor.rgb *= (luminance + warpBrightPassThreshold);
        }
    }
    else {
        // === SIMPLE FEEDBACK RENDERING PATH ===
        finalColor = sampleSimpleFeedback(vTexCoord);
    }
    
    // Apply video color and maintain existing color controls
    fragColor = finalColor * videoColor;
}