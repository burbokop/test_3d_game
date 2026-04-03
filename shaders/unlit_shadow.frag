#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 pixelGlobalCoordinates;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 3) uniform sampler2D texSampler;

layout(binding = 0, set = 0) uniform UniformBufferObject {
    mat4 transformation;
    float time;
    vec2 mouse;
} global;

struct PointLightUniformBufferObject {
    vec4 position;
    vec4 color;
};

layout(binding = 0, set = 1) uniform LightingUniformBufferObject {
    PointLightUniformBufferObject lights[64];
    uint lightsCount;
    float ambient;
} lighting;

void main() {
    const float depth = 1. - texture(texSampler, fragTexCoord).r;
    outColor = vec4(depth, depth, depth, 1);
}
