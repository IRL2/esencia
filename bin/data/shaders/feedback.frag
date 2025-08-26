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
uniform float zoom; // 1.0 = no zoom, >1.0 = zoom in, <1.0 = zoom out

in vec2 vTexCoord;
out vec4 fragColor;

// function to calculate warp displacement based on current and previous frames
vec2 calculateWarpDisplacement(vec2 texCoord) {
    vec2 centeredCoord = (vTexCoord - 0.5) / zoom + 0.5;
    vec2 scaledCoord = centeredCoord * warpDetail;
    // vec2 scaledCoord = texCoord * warpDetail;

    vec2 texelSize = 1.0 / textureSize(currentFrame, 0);
    
    vec2 spreadX = vec2(warpSpreadX * texelSize.x, 0.0);
    vec2 spreadY = vec2(0.0, warpSpreadY * texelSize.y);
    
    // sample neighboring positions
    vec2 left = scaledCoord - spreadX;
    vec2 right = scaledCoord + spreadX;
    vec2 top = scaledCoord - spreadY;
    vec2 bottom = scaledCoord + spreadY;
    
    // get current frame gradients (using red channel for luminance-based warping)
    float current_Center = texture(currentFrame, scaledCoord).r;
    float current_Right = texture(currentFrame, right).r;
    float current_Bottom = texture(currentFrame, bottom).r;
    
    // calculate new displacement based on gradients
    vec2 newDisplacement = vec2(
        (current_Center - current_Right) * warpVariance,
        (current_Center - current_Bottom) * warpVariance
    );
  
    
    // displacement from the previous frame gradients
    float prev_Center = texture(previousFrame, scaledCoord).r;
    float prev_Left = texture(previousFrame, left).r;
    float prev_Right = texture(previousFrame, right).r;
    float prev_Top = texture(previousFrame, top).r;
    float prev_Bottom = texture(previousFrame, bottom).r;
    
    // propagation by using neighboring gradients from previous frame
    vec2 propagatedDisplacement = vec2(
        (prev_Center * warpPropagationPersistence) + 
        ((prev_Left + prev_Right + prev_Top + prev_Bottom) * warpPropagation * 0.25),
        (prev_Center * warpPropagationPersistence) + 
        ((prev_Left + prev_Right + prev_Top + prev_Bottom) * warpPropagation * 0.25)
    );
    
    vec2 finalDisplacement = newDisplacement + propagatedDisplacement * 0.2;
    
    finalDisplacement = clamp(finalDisplacement, vec2(-2.0), vec2(2.0));
    
    return finalDisplacement;
}

vec4 sampleWarped(sampler2D tex, vec2 texCoord) {
    vec2 displacement = calculateWarpDisplacement(texCoord);
    vec2 warpedCoord = texCoord + displacement;
    
    warpedCoord = clamp(warpedCoord, vec2(0.0), vec2(1.0));
    
    return texture(tex, warpedCoord);
}

void main() {

    vec4 warpedCurrent = sampleWarped(currentFrame, vTexCoord);
    
    vec4 previousColor = texture(previousFrame, vTexCoord);
    
    vec4 finalColor = mix(warpedCurrent, previousColor, feedbackAmount * 0.1);
    
    if (warpBrightPassThreshold > 0.0) {
        vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);
        float luminance = dot(finalColor.rgb, luminanceVector);
        luminance = max(0.0, luminance - warpBrightPassThreshold);
        finalColor.rgb *= (luminance + warpBrightPassThreshold);
    }
    
    fragColor = finalColor * videoColor;
}