#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 cameraMatrix;

out vec3 vertexColor;
out vec2 TexCoord;

void main()
{
    gl_Position = cameraMatrix * vec4(aPos, 1.0);
    vertexColor = aColor;
    TexCoord = aTexCoord;
}
