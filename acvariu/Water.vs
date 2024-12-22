#version 330 core
layout(location = 0) in vec3 aPos;   // Vertex position
layout(location = 1) in vec3 aNormal; // Normal vector

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;   // Pass world position to fragment shader
out vec3 Normal;    // Pass normals to fragment shader

void main()
{
    vec3 modifiedPos = aPos;

    // No sinusoidal ripple effect anymore

    FragPos = vec3(model * vec4(modifiedPos, 1.0)); // World space position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normals to world space
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
