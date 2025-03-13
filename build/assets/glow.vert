#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec2 FragPos;  // Screen-space coordinates

uniform vec2 uScreenSize; // Add uniform for screen size

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;

    // Convert from NDC (-1 to 1) to screen coordinates (0 to width/height)
    FragPos = (aPos + 1.0) * 0.5 * uScreenSize;
}
