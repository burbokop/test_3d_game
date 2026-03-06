#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform UniformBufferObject {
        mat4 transformation;
        float time;
        float _time;
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

layout(binding = 0, set = 2) uniform OM {
    mat4 model;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 globalCoordinates;
layout(location = 3) out vec3 normal;

// float random (vec3 st) {
//     return fract(sin(dot(st.xyz, vec3(12.9898,78.233, 3.1234))) * 43758.5453123);
// }

void main() {
    //gl_Position = ubo.proj * ubo.view * ubo.model * object.model * vec4(inPosition, 0.0, 1.0);

    mat4 tr = global.transformation;
    mat4 model = object.model;
    mat4 comb = tr * model;

    mat4 iden = mat4(
        1.,0.,0.,0,
        0.,1.,0.,0,
        0.,0.,1.,0,
        0.,0.,0.,1
    );

    gl_Position = comb * vec4(inPosition, 1.0);
    globalCoordinates = (object.model * vec4(inPosition, 1.0)).xyz;
    normal = inNormal;


    // fragColor = inColor * mod(random(inPosition), 1.);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
