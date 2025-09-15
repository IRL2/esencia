#version 150

in vec2 vTexCoord;
out vec4 outputColor;

uniform sampler2DRect tex0;
uniform float uFade; // 0 = no fade, 1 = full fade

void main() {
    vec4 color = texture(tex0, vTexCoord);
    // Fade out alpha and color for trail effect
    color.rgb *= (1.0 - uFade);
    color.a *= (1.0 - uFade);
    outputColor = color;
}