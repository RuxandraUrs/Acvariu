#pragma once
#ifndef FISH_H
#define FISH_H

#include <glm/glm.hpp>
#include <GL/glut.h>

#include "Model.h"

class Fish {
private:
    Model fishModel;
    glm::vec3 position;

public:
    Fish(const std::string& modelPath) : fishModel(modelPath, false) {}

    void Update(float deltaTime) {
        // Update position based on animation logic
    }

    void Render(Shader& shader) {
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
        shader.setMat4("model", modelMatrix);
        fishModel.Draw(shader);
    }
};
