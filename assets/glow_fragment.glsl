#version 330 core
in vec2 FragPos;
out vec4 FragColor;

uniform vec2 uRectCenter;
uniform vec2 uRectSize;
uniform float uGlowSize;
uniform vec3 uGlowColor;
uniform vec3 uColor;

void main() {
    vec2 dist = abs(FragPos - uRectCenter);
    vec2 edgeDist = dist - uRectSize;
    float maxEdgeDist = max(edgeDist.x, edgeDist.y);
    
    if (maxEdgeDist <= 0.0) {
        FragColor = vec4(uColor, 1.0);  // Inside: solid color
    } else {
        float glowIntensity = smoothstep(uGlowSize, 0.0, maxEdgeDist); // Invert logic for clarity
        if (glowIntensity > 0.0) {
            FragColor = vec4(uGlowColor, glowIntensity);  // Glow with alpha
        } else {
            FragColor = vec4(0.0, 0.0, 0.0, 0.0);  // Transparent outside
        }
    }
}