layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 eye;

    vec3 sunDir;
    vec3 sunColor;
} ubo;
