#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in float instanceSize; // Attribute 1: particle size

void main() {
    gl_Position = modelViewProjectionMatrix * position;
    gl_PointSize = instanceSize;
}
