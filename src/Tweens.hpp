#include "VoltaFramework.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>

static float easeLinear(float t) {
    return t;
}

static float easeSine(float t) {
    return sin(t * M_PI_2);
}

static float easeQuad(float t) {
    return t * t;
}

static float easeCubic(float t) {
    return t * t * t;
}

static float easeQuart(float t) {
    return t * t * t * t;
}

static float easeQuint(float t) {
    return t * t * t * t * t;
}

static float easeExponential(float t) {
    return (t == 0.0f) ? 0.0f : pow(2, 10 * (t - 1));
}

static float easeCircular(float t) {
    return 1 - sqrt(1 - t * t);
}

static float easeBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return c3 * t * t * t - c1 * t * t;
}

static float easeBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1 / d1) {
        return n1 * t * t;
    } else if (t < 2 / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5 / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

static float easeElastic(float t) {
    const float c4 = (2 * M_PI) / 3;
    return (t == 0) ? 0 : (t == 1) ? 1 : pow(2, -10 * t) * sin((t * 10 - 0.75) * c4) + 1;
}

static float applyEasingDirection(float t, float eased, const char* direction) {
    if (strcmp(direction, "out") == 0) {
        return eased;
    } else if (strcmp(direction, "in") == 0) {
        return 1 - eased;
    } else if (strcmp(direction, "inout") == 0) {
        return t < 0.5 ? (1 - eased) / 2 : (1 + eased) / 2;
    }
    return eased;
}