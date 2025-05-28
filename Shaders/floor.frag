#version 330 core

in vec3 vertexColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D floorTexture;

void main()
{
    vec4 texColor = texture(floorTexture, TexCoord);
    FragColor = texColor * vec4(vertexColor, 1.0); // combineer met kleur
}
