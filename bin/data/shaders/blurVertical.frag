#version 150

in vec2 vTexCoord;
out vec4 outputColor;

uniform sampler2DRect tex0;
uniform float sigma; // This is from the GUI parameter

void main() {
    // Calculate radius from sigma
    float radiusF = sigma * 3.0;           
    int radius = int(floor(radiusF));

    // If sigma is really small, radius might be 0 => no blur
    if (radius < 1) {
        // No blur => just fetch the source tex
        outputColor = texture(tex0, vTexCoord);
        return;
    }

    vec2 uv = vTexCoord;
    float wsum = 0.0;
    vec4 sum = vec4(0.0);

    // Loop from -radius to +radius, sampling vertically
    for(int i = -radius; i <= radius; i++){
        float y = float(i);

        // Gaussian weight
        float weight = exp(-0.5 * (y * y) / (sigma * sigma));

        // Sample in vertical direction
        vec4 col = texture(tex0, uv + vec2(0, y));

        sum += col * weight;
        wsum += weight;
    }

    // Normalize
    vec4 blurred = sum / wsum;
    outputColor = blurred;
}
