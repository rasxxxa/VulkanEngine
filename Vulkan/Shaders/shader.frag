#version 450

layout(location = 0) out vec4 outColor; // final output color must have loca
layout(location = 0) in vec3 fragColor;
layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 1) in vec2 fragTex;

void main()
{
   outColor = texture(textureSampler, fragTex);
}