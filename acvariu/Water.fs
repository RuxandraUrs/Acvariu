#version 330 core
in vec3 FragPos;  // World space position
in vec3 Normal;   // World space normal

uniform vec3 lightPos;   // Light position
uniform vec3 viewPos;    // Camera position
uniform vec3 lightColor; // Light color
uniform vec3 objectColor; // Base water color
uniform sampler2D texture1; // Water texture

out vec4 FragColor;

void main()
{
    // Ambient lighting
    float ambientStrength = 0.3;
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
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;

    // Sample the texture
    vec2 texCoord = FragPos.xz * 0.1; // Adjust texture scaling for a natural look
    vec3 textureColor = texture(texture1, texCoord).rgb;

    // Combine results
    vec3 finalColor = mix(objectColor, textureColor, 0.7); // Blend base color and texture
    finalColor += ambient + diffuse + specular;

    // Set transparency
    float alpha = 0.3; // Adjust this for desired transparency
    FragColor = vec4(finalColor, alpha); // Use alpha for transparency
}
