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
    float maxEdgeDist = length(max(dist - uRectSize, vec2(0.0)));
    
    if (maxEdgeDist <= 0.0) {
        FragColor = vec4(uColor, 1.0);  // Inside: solid color
    } else {
        // Invert the smoothstep to increase intensity as distance grows (within glow range)
        float glowIntensity = 1.0 - smoothstep(0.0, uGlowSize, maxEdgeDist);
        if (glowIntensity > 0.0) {
            FragColor = vec4(uGlowColor, glowIntensity);  // Outside: glow
        } else {
            FragColor = vec4(0.0, 0.0, 0.0, 0.0);  // Transparent outside glow range
        }
    }
}