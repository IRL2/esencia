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

