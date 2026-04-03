#version 450
#extension GL_ARB_separate_shader_objects : enable

float rand(vec3 seed) {
    return fract(sin(dot(seed, vec3(12.9898, 78.233, 48.782))) * 437.5453);
}

layout(location = 0) in vec3 inputDirection;
layout(location = 0) out vec4 outputColor;

const float precisionCoef = 800;
const float chance = 0.000000001;

void main() {
    const vec3 direction = normalize(inputDirection);
    const float random = abs(rand(floor(direction * precisionCoef) / precisionCoef));
    outputColor = random < chance ? vec4(1) : vec4(0);
}
