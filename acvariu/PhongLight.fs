#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform float KA;
uniform float KD;
uniform float KS;
  
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float transparency;

void main()
{
    vec3 ambient = KA * lightColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = KD * max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    
    float spec = KS * pow(max(dot(viewDir, reflectDir), 0.0f), 16);
    vec3 specular = spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    
    FragColor = vec4(result, transparency);
}