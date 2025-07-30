#version 150

uniform sampler2D tex0;
uniform vec4 videoColor;
uniform float time;
uniform vec2 resolution;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    vec4 texColor = texture(tex0, vTexCoord);
    
    //  existing color and alpha controls
    fragColor = texColor * videoColor;
    
}




// #version 150

// uniform sampler2D tex0;
// uniform vec4 videoColor;
// uniform float time;
// uniform vec2 resolution;

// in vec2 vTexCoord;
// out vec4 fragColor;

// // Simple 2D noise function for metallic distortion
// float hash(vec2 p) {
//     return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
// }

// float noise(vec2 p) {
//     vec2 i = floor(p);
//     vec2 f = fract(p);
//     float a = hash(i);
//     float b = hash(i + vec2(1.0, 0.0));
//     float c = hash(i + vec2(0.0, 1.0));
//     float d = hash(i + vec2(1.0, 1.0));
//     vec2 u = f * f * (3.0 - 2.0 * f);
//     return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
// }

// void main() {
//     float alphaThreshold = 0.05; // edge threshold for alpha
//     float auraWidth = 0.1;      // width of aura band (in UV units)
    
//     // Sample alpha at current pixel
//     float alpha = texture(tex0, vTexCoord).a;
    
//     // Find edge: check alpha at nearby points
//     float edge = 0.0;
//     float offsets[4] = float[4](0.0, 1.0, -1.0, 0.0);
//     for (int i = 0; i < 4; ++i) {
//         vec2 offset = vec2(offsets[i], offsets[(i+1)%4]) * auraWidth * 0.5;
//         float neighborAlpha = texture(tex0, vTexCoord + offset).a;
//         if ((alpha > alphaThreshold && neighborAlpha <= alphaThreshold) ||
//             (alpha <= alphaThreshold && neighborAlpha > alphaThreshold)) {
//             edge = 1.0;
//         }
//     }
    
//     // Calculate aura mask: only apply ripple in a band around the edge
//     float auraMask = 0.0;
//     if (edge > 0.5) {
//         // Calculate distance from edge (approximate by sampling outwards)
//         float minDist = 1.0;
//         for (float r = 0.0; r <= auraWidth; r += auraWidth/4.0) {
//             float a = texture(tex0, vTexCoord + vec2(r,0)).a;
//             if ((alpha > alphaThreshold && a <= alphaThreshold) ||
//                 (alpha <= alphaThreshold && a > alphaThreshold)) {
//                 minDist = min(minDist, r);
//             }
//             a = texture(tex0, vTexCoord + vec2(-r,0)).a;
//             if ((alpha > alphaThreshold && a <= alphaThreshold) ||
//                 (alpha <= alphaThreshold && a > alphaThreshold)) {
//                 minDist = min(minDist, r);
//             }
//             a = texture(tex0, vTexCoord + vec2(0,r)).a;
//             if ((alpha > alphaThreshold && a <= alphaThreshold) ||
//                 (alpha <= alphaThreshold && a > alphaThreshold)) {
//                 minDist = min(minDist, r);
//             }
//             a = texture(tex0, vTexCoord + vec2(0,-r)).a;
//             if ((alpha > alphaThreshold && a <= alphaThreshold) ||
//                 (alpha <= alphaThreshold && a > alphaThreshold)) {
//                 minDist = min(minDist, r);
//             }
//         }
//         auraMask = smoothstep(auraWidth, 0.0, minDist);
//     }
    
//     // Ripple parameters
//     float frequency = 18.0;
//     float speed = 2.0;
//     float amplitude = 0.018;
    
//     // Ripple only if in aura
//     vec2 uv = vTexCoord;
//     if (auraMask > 0.0) {
//         float ripple = sin((vTexCoord.x + vTexCoord.y) * frequency - time * speed) * amplitude * auraMask;
//         uv += normalize(vec2(0.5) - vTexCoord) * ripple;
//     }
    
//     // --- Feedback effect ---
//     // Offset the coordinates in a swirling pattern for feedback illusion
//     float feedbackStrength = 0.012;
//     float swirl = 2.0 * 3.14159 * (0.2 + 0.1 * sin(time * 0.7));
//     vec2 center = vec2(0.5, 0.5);
//     vec2 toCenter = uv - center;
//     float angle = swirl * length(toCenter);
//     float s = sin(angle);
//     float c = cos(angle);
//     vec2 rotated = vec2(
//         toCenter.x * c - toCenter.y * s,
//         toCenter.x * s + toCenter.y * c
//     );
//     vec2 feedbackUV = center + rotated * (1.0 + feedbackStrength * (0.5 + 0.5 * sin(time + length(toCenter) * 8.0)));
    
//     // Blend original and feedback sample
//     vec4 texColor = mix(texture(tex0, uv), texture(tex0, feedbackUV), 0.35);
    
//     if (alpha > alphaThreshold) {
//         // Metallic normal perturbation
//         float n = noise(vTexCoord * 18.0 + time * 0.7);
//         float n2 = noise(vTexCoord * 24.0 - time * 0.5);
//         vec2 normal = normalize(vec2(n - 0.5, n2 - 0.5));

//         // Simulate a light direction
//         vec2 lightDir = normalize(vec2(0.7, 0.6));
//         float diffuse = max(dot(normal, lightDir), 0.0);

//         // Specular highlight
//         float specular = pow(max(dot(normal, lightDir), 0.0), 32.0);

//         // Color modulation for metallic look
//         vec3 baseColor = texColor.rgb * videoColor.rgb;
//         vec3 metalColor = mix(baseColor, vec3(0.85, 0.88, 0.92), 0.7); // silvery
//         vec3 color = metalColor * (0.5 + 0.5 * diffuse) + specular * vec3(1.0, 1.0, 1.0);

//         fragColor = vec4(color, texColor.a * videoColor.a);
//     } else {
//         // Transparent parts remain unchanged
//         fragColor = texColor * videoColor;
//     }
// }