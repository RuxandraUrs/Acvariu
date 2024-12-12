#version 330 core
layout(location = 0) in vec3 aPos;  // Vertex position
layout(location = 1) in vec3 aNormal; // Normal vector

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Time for animation

out vec3 FragPos;   // Pass world position to fragment shader
out vec3 Normal;    // Pass normals to fragment shader

void main()
{
    vec3 modifiedPos = aPos;

    // Apply ripple effect only to the top face (check Y-normal)
    if (aNormal.y > 0.99) {
        modifiedPos.y += sin(aPos.x * 10.0 + time * 3.0) * 0.05; // Ripple effect
        modifiedPos.y += sin(aPos.z * 10.0 + time * 3.0) * 0.05; // Cross-direction ripples
    }

    FragPos = vec3(model * vec4(modifiedPos, 1.0)); // World space position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normals to world space
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
