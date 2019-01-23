#version 450

// Attributes
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Color;

// Uniforms
//layout(binding = 0) uniform UniformBufferObject
//{
//    mat4 world;
//    mat4 view;
//    mat4 proj;
//} ubo;

// Varyings
layout(location = 0) out vec4 v_Color;

void main()
{
    gl_Position = /*ubo.world **/ vec4( a_Position, 0.0, 1.0 );

    v_Color = a_Color;
}
