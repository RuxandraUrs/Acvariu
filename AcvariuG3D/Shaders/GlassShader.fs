#version 330 core
in vec3 FragPos; // Fragment position
in vec3 Normal;  // Normal at the fragment

uniform vec3 objectColor;  // Object color (blue)
uniform float transparency; // Transparency value (e.g., 0.5 for 50%)

void main() {
    // Set the fragment color using the uniform values
    vec4 color = vec4(objectColor, transparency);
    gl_FragColor = color;
}
