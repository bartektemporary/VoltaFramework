#version 330 core
in vec3 fragColor; // Base color from framework (1.0, 1.0, 1.0)
in vec2 fragPos;   // Rectangle center position in [0, 1] space
out vec4 FragColor;

uniform float uTime;         // Time for animation
uniform vec2 uRectPos;       // Center position of rectangle in [0, 1] space
uniform vec2 uRectSize;      // Size of rectangle in [0, 1] space

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(800.0, 600.0); // Normalize to [0, 1], adjust to window size

    // Calculate rectangle bounds
    vec2 rectMin = uRectPos - uRectSize * 0.5;
    vec2 rectMax = uRectPos + uRectSize * 0.5;
    
    // Check if we're inside the rectangle
    bool inside = uv.x >= rectMin.x && uv.x <= rectMax.x &&
                  uv.y >= rectMin.y && uv.y <= rectMax.y;
    
    // Calculate distance to rectangle edge for glow
    vec2 distToEdge = max(vec2(0.0), max(rectMin - uv, uv - rectMax));
    float dist = length(distToEdge);
    
    // Glow effect with time-based pulsing
    float glowStrength = 0.05 / (dist + 0.01 + 0.005 * sin(uTime));
    glowStrength = clamp(glowStrength, 0.0, 1.0);
    
    // Scale the x-position range (0.375 to 0.625) to (0 to 1)
    float minX = 0.375; // Leftmost position
    float maxX = 0.625; // Rightmost position
    float t = clamp((fragPos.x - minX) / (maxX - minX), 0.0, 1.0); // Map to 0-1
    
    // Pure red-to-blue transition
    vec3 dynamicColor = vec3(1.0 - t, 0.0, t); // Red (1, 0, 0) to Blue (0, 0, 1)
    
    // Apply base color
    vec3 finalColor = fragColor * dynamicColor;
    
    if (inside) {
        FragColor = vec4(finalColor, 1.0); // Full color inside
    } else {
        FragColor = vec4(finalColor * glowStrength, glowStrength); // Glow outside
    }
}