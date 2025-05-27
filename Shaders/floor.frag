#version 330 core
#define GLM_ENABLE_EXPERIMENTAL
in vec3 vertexColor;
out vec4 FragColor;


void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
