#version 330 core
layout (location = 0) in vec2 aPos;

uniform vec3 uColor; // Base color from framework
uniform bool uUseTexture; // Provided by framework
uniform vec2 uRectPos; // Rectangle center position in [0, 1] space

out vec3 fragColor;
out vec2 fragPos; // Pass position to fragment shader

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    fragColor = uColor; // Pass base color
    fragPos = uRectPos; // Pass rectangle position
}