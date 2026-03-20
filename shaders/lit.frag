
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
    const vec4 texColor = texture(texSampler, fragTexCoord);

    //outColor = (texColor * 0.8 + vec4(fragTexCoord, 0.0, 1.0) * 0.2);

    //vec3 lightsSumColor = fragColor;

    const vec3 materialColor = texColor.xyz;

    vec3 lightsSumColor = materialColor * lighting.ambient;
    for(uint i = 0; i < lighting.lightsCount; ++i) {
        const vec3 delta = lighting.lights[i].position.xyz - pixelGlobalCoordinates;
        const float intencity = lighting.lights[i].color.w / pow(length(delta), 2);
        const float d = dot(normalize(normal), normalize(delta));

        const float dd = d * intencity;
        // const float dd = max(d * intencity, ambient);

        lightsSumColor += mix(materialColor, lighting.lights[i].color.xyz, dd);
    }
    lightsSumColor /= (lighting.lightsCount + 1);



    // outColor = vec4(mix(lightsSumColor, fragColor, 0.5), 1.0);


    outColor = vec4(lightsSumColor, 1.);
    //outColor = vec4(0, intencity, 0, 1.);

    //outColor = vec4(normalize(delta), 1.);

    // if(lighting.lightsCount == 1) {
    //     outColor = vec4(1., 0, 0, 1.);
    // } else {
    //     outColor = vec4(0, 1., 0, 1.);
    // }

    //         outColor = vec4(1.0,0,0, 1.0);

    //outColor = texture(texSampler, fragTexCoord);
    //outColor.w = 0.3;
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
}
