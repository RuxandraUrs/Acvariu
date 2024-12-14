#version 330 core

layout(location = 0) in vec3 aPos;    // Vertex position
layout(location = 1) in vec3 aNormal; // Vertex normal

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos; // Fragment position in world space
out vec3 Normal;  // Normal vector in world space

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // Correct normal for scaling
    gl_Position = projection * view * vec4(FragPos, 1.0);
}