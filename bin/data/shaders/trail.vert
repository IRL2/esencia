#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in float size;

uniform mat4 modelViewProjectionMatrix;

out float pointSize; // Pass particle size to fragment shader

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
    gl_PointSize = size; // Set the actual point size that will be rendered
    pointSize = size; // Pass size to fragment shader for additional processing
}