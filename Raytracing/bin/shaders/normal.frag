#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

layout(binding = 0) uniform sampler2D Diffuse;

void main()
{           
    FragColor = texture(Diffuse, TexCoords );
}