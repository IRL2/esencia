#version 150

in vec2 vTexCoord;
out vec4 outputColor;

uniform sampler2DRect tex0;
uniform float sigma; // This is from the GUI parameter

void main(){
    // Instead of a fixed KERNEL_RADIUS, calculate radius from sigma
    // "3.0" is a common approximation: ~99% of a Gaussian distribution fits in ±3*sigma
    float radiusF = sigma * 3.0;           
    int radius = int(floor(radiusF));      

    // If sigma is really small, radius might be 0 => no blur
    // If sigma is large (e.g., 100), radius becomes ~300 => big loop => performance cost
    if (radius < 1) {
        // No blur => just fetch the source tex
        outputColor = texture(tex0, vTexCoord);
        return;
    }

    vec2 uv = vTexCoord;

    float wsum = 0.0;
    vec4 sum = vec4(0.0);

    // Loop from -radius to +radius
    for(int i = -radius; i <= radius; i++){
        float x = float(i);

        // Gaussian weight
        float weight = exp(-0.5 * (x*x)/(sigma*sigma));

        // Sample horizontally
        vec4 col = texture(tex0, uv + vec2(x, 0));

        sum += col * weight;
        wsum += weight;
    }

    // Normalize
    vec4 blurred = sum / wsum;

    // If you only use the red channel, you can store it in .r. But here we store in RGBA
    outputColor = blurred;
}
