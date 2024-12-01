#pragma once
#ifndef AQUARIUM_H
#define AQUARIUM_H

#include <vector>
#include "Fish.h"
#include "Plant.h"
#include "Decor.h"
#include "Bubble.h"
#include "Light.h"

#include "Model.h"

class Aquarium {
private:
    std::vector<Model> models; // Use Model class for each aquarium element
    Shader lightingShader;
    Shader lampShader;

public:
    Aquarium() : lightingShader("path/to/vertex_shader.vs", "path/to/fragment_shader.fs"),
        lampShader("path/to/lamp_shader.vs", "path/to/lamp_shader.fs") {}

    void InitScene() {
        models.push_back(Model("path/to/fish_model.obj", false));
        models.push_back(Model("path/to/decor_model.obj", false));
        // Add other models
    }

    void Render(const Camera& camera) {
        lightingShader.use();
        lightingShader.setMat4("projection", camera.GetProjectionMatrix());
        lightingShader.setMat4("view", camera.GetViewMatrix());

        for (const auto& model : models) {
            glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)); // Example scaling
            lightingShader.setMat4("model", modelMatrix);
            model.Draw(lightingShader);
        }
    }
};

#endif // AQUARIUM_H
