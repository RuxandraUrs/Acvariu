#version 330 core

in vec3 FragPos;  // Fragment position in world space
in vec3 Normal;   // Normal vector in world space

out vec4 FragColor;

uniform vec3 lightPos;    // Light source position
uniform vec3 viewPos;     // Camera position
uniform vec3 objectColor; // Object base color
uniform vec3 lightColor;  // Light color

void main()
{
    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16); // Adjust shininess
    vec3 specular = specularStrength * spec * lightColor;

    // Combine lighting effects
    vec3 result = (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result, 1.0);
}