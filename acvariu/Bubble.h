#pragma once
#ifndef BUBBLE_H
#define BUBBLE_H

#include <glm/glm.hpp>
#include <GL/glut.h>

class Bubble {
private:
    glm::vec3 position;
    float size;
    float speed;

public:
    Bubble(glm::vec3 pos, float sz, float spd);
    void Move(float deltaTime);
    void RenderBubble();
};

#endif // BUBBLE_H
