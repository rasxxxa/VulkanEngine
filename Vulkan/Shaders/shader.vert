#version 450 // use GLSL 4.5

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

layout(binding = 0) uniform ViewProjection
{
   mat4 projection;
   mat4 view;
} viewprojection;

// NOT IN USE, LEFT FOR REFERENCE
layout(binding = 1) uniform Model
{
   mat4 model;
} model;

layout(push_constant) uniform PushModel
{
  mat4 model;
} pushModel;

layout(location = 0) out vec3 fragCol;

void main()
{
    gl_Position = viewprojection.projection * viewprojection.view * pushModel.model * vec4(pos, 1.0);
	fragCol = col;
}