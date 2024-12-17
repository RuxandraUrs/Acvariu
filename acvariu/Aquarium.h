#ifndef AQUARIUM_H
#define AQUARIUM_H

#include <vector>
#include <string>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Camera.h"
#include <GL/glew.h>

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 
#include <locale>
#include <codecvt>
#include <cmath>
#include <stb_image.h>


#define M_PI 3.14159265358979323846


class Aquarium {
public:
    unsigned int VBO1, VAO1, VBO2, VAO2, VBO3, VAO3, VAO4, VBO4;
    unsigned int algaeVAO, algaeVBO;

    Aquarium();
    ~Aquarium();

    void RenderBottom(Shader& shader, const Camera& camera);
    void RenderGlass(Shader& shader, const Camera& camera);
    void RenderWater(Shader& shader, const Camera& camera, float time, const std::string& texturePath);
    void RenderCuboid(Shader& shader, const Camera& camera);
    void RenderAlgae(Shader& shader, const Camera& camera, float time, glm::vec3 basePosition, int numAlgae, glm::vec3 color);
    unsigned int LoadTexture(const std::string& filePath);
	

    void InitializeAlgaeBuffer();

private:
    void InitializeBuffers();
    void InitializeWaterBuffer();
    void InitializesandBuffers();
    void Cleanup();
};


#endif