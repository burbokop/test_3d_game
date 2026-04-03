#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0, std140) uniform Global {
    mat4 transformation;
    float time;
    vec2 mouse;
    vec3 cameraPosition;
    uint mode;
} global;

layout(location = 0) in vec3 inputPosition;

layout(location = 0) out vec3 outputDirection;

void main() {
    gl_Position = vec4(inputPosition, 1.0);
    const vec4 positionInWorldSpace = inverse(global.transformation) * vec4(inputPosition, 1);
    outputDirection = (positionInWorldSpace.xyz / positionInWorldSpace.w) - global.cameraPosition;
}
