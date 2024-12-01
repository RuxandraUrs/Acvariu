#pragma once
#ifndef DECOR_H
#define DECOR_H

#include <glm/glm.hpp>
#include <GL/glut.h>
#include <string>

class Decor {
private:
    glm::vec3 position;
    std::string type;
    glm::vec3 color;

public:
    Decor(glm::vec3 pos, const std::string& type, glm::vec3 col);
    void RenderDecor();
};

#endif // DECOR_H
