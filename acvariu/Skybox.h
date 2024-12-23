#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm.hpp>
#include "Shader.h"

class Skybox {
public:
    // Constructor
    Skybox(const std::vector<std::string>& faces);

    // Destructor
    ~Skybox();

    // Render the skybox
    void Render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int skyboxVAO, skyboxVBO, cubemapTexture;

    // Load cubemap textures
    unsigned int LoadCubemap(const std::vector<std::string>& faces);

    // Initialize the VAO and VBO
    void Initialize();
};
