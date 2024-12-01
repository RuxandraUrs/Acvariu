#pragma once
#ifndef PLANT_H
#define PLANT_H

#include <glm/glm.hpp>
#include <GL/glut.h>

class Plant {
private:
    glm::vec3 position;
    glm::vec3 color;
    float animationPhase;

public:
    Plant(glm::vec3 pos, glm::vec3 col);
    void Animate(float deltaTime);
    void RenderPlant();
};

#endif // PLANT_H
