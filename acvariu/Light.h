#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <GL/glut.h>

class Light {
private:
    glm::vec3 ambientIntensity;
    glm::vec3 directionalPosition;
    glm::vec3 directionalIntensity;

public:
    Light(glm::vec3 ambient, glm::vec3 dirPos, glm::vec3 dirIntensity);
    void UpdateLighting(float timeOfDay);
    void ApplyLighting();
};

#endif // LIGHT_H
