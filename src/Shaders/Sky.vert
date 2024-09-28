#version 450

#include "include/VertexP.glsl"
#include "include/UniformBufferObject.glsl"

void main()
{
    gl_Position = ubo.proj * vec4(inPosition, 1.0);
}
