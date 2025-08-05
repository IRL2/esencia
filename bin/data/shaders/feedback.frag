#version 150

uniform sampler2D currentFrame;
uniform sampler2D previousFrame;
uniform float time;
uniform vec2 resolution;
uniform float feedbackAmount;
uniform vec4 videoColor;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    vec2 uv = vTexCoord;
    
    vec2 center = vec2(0.5, 0.5);
    vec2 fromCenter = uv - center;
    float dist = length(fromCenter);
    
    float breathe = sin(time * 0.8) * 0.003;
    float radial = sin(time * 0.4 + dist * 8.0) * 0.002;
    
    vec2 offset = vec2(
        sin(time * 0.2 + uv.y * 6.0) * 0.001,
        cos(time * 0.3 + uv.x * 4.0) * 0.001
    );
    
    vec2 radialOffset = normalize(fromCenter) * (breathe + radial);
    vec2 feedbackUV = uv + offset + radialOffset;
    
    vec4 current = texture(currentFrame, uv);
    vec4 previous = texture(previousFrame, feedbackUV);
    
    float prevAlpha = previous.a;
    
    float distFromEdge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    float auraIntensity = 1.0 + (1.0 - smoothstep(0.0, 0.3, distFromEdge)) * 0.5;
    
    float fadeAmount = 0.99 * auraIntensity;
    previous.rgb *= fadeAmount;
    
    float baseBlend = 0.9;
    float edgeBoost = (1.0 - smoothstep(0.0, 0.2, distFromEdge)) * 0.1;
    float blendAmount = (baseBlend + edgeBoost) * smoothstep(0.0, 0.1, prevAlpha);
    
    vec3 result = mix(current.rgb, previous.rgb, blendAmount);
    
    float alphaBoost = 1.0 + (1.0 - smoothstep(0.0, 0.4, distFromEdge)) * 0.3;
    float finalAlpha = max(current.a, previous.a * 0.95) * alphaBoost;
    
    fragColor = vec4(result, finalAlpha) * videoColor;
}