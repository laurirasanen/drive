layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 clipToWorld;
    vec3 eye;

    vec4 viewport;

    vec3 sunDir;
    vec3 sunColor;
} ubo;
