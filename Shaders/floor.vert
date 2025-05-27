#version 330 core
#define GLM_ENABLE_EXPERIMENTAL
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;


uniform mat4 cameraMatrix;

out vec3 vertexColor;

void main()
{
    gl_Position = cameraMatrix * vec4(aPos, 1.0);
    vertexColor = aColor;
}
