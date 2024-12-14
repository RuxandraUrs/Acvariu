#include <Windows.h>
#include <locale>
#include <codecvt>
#include <cmath>


#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Model.h"
#include <stb_image.h>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")


const unsigned int SCR_WIDTH = 1900;
const unsigned int SCR_HEIGHT = 1800;

glm::vec3 linearFishPosition(0.0f, 0.8f, 0.0f); // Poziția inițială a peștelui
glm::vec3 linearFishDirection(1.0f, 0.0f, 0.0f); // Direcția inițială (merge pe axa x)
float aquariumBoundary = 3.0f; // Dimensiunea acvariului
bool isSinusoidal = false; // Flag pentru rotație sinusoidală
float sinusoidalTime = 0.0f; // Timp pentru rotație sinusoidală

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



enum ECameraMovementType
{
	UNKNOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

unsigned int LoadTexture(const std::string& filePath)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Failed to load texture at path: " << filePath << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

class Camera
{
private:
	// Default camera values
	const float zNEAR = 0.1f;
	const float zFAR = 200.f;
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float FOV = 45.0f;
	glm::vec3 startPosition;

public:

	Camera(const int width, const int height, const glm::vec3& position)
	{
		startPosition = position;
		Set(width, height, position);
	}

	void Set(const int width, const int height, const glm::vec3& position)
	{
		this->isPerspective = true;
		this->yaw = YAW;
		this->pitch = PITCH;

		this->FoVy = FOV;
		this->width = width;
		this->height = height;
		this->zNear = zNEAR;
		this->zFar = zFAR;

		this->worldUp = glm::vec3(0, 1, 0);
		this->position = position;

		lastX = width / 2.0f;
		lastY = height / 2.0f;
		bFirstMouseMove = true;

		UpdateCameraVectors();
	}

	void Reset(const int width, const int height)
	{
		Set(width, height, startPosition);
	}

	void Reshape(int windowWidth, int windowHeight)
	{
		width = windowWidth;
		height = windowHeight;

		// define the viewport transformation
		glViewport(0, 0, windowWidth, windowHeight);
	}

	const glm::mat4 GetViewMatrix() const
	{
		// Returns the View Matrix
		return glm::lookAt(position, position + forward, up);
	}

	const glm::vec3 GetPosition() const
	{
		return position;
	}

	const glm::mat4 GetProjectionMatrix() const
	{
		glm::mat4 Proj = glm::mat4(1);
		if (isPerspective) {
			float aspectRatio = ((float)(width)) / height;
			Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
		}
		else {
			float scaleFactor = 2000.f;
			Proj = glm::ortho<float>(
				-width / scaleFactor, width / scaleFactor,
				-height / scaleFactor, height / scaleFactor, -zFar, zFar);
		}
		return Proj;
	}

	void ProcessKeyboard(ECameraMovementType direction, float deltaTime)
	{
		float velocity = (float)(cameraSpeedFactor * deltaTime);
		switch (direction) {
		case ECameraMovementType::FORWARD:
			position += forward * velocity;
			break;
		case ECameraMovementType::BACKWARD:
			position -= forward * velocity;
			break;
		case ECameraMovementType::LEFT:
			position -= right * velocity;
			break;
		case ECameraMovementType::RIGHT:
			position += right * velocity;
			break;
		case ECameraMovementType::UP:
			position += up * velocity;
			break;
		case ECameraMovementType::DOWN:
			position -= up * velocity;
			break;
		}
	}

	void MouseControl(float xPos, float yPos)
	{
		if (bFirstMouseMove) {
			lastX = xPos;
			lastY = yPos;
			bFirstMouseMove = false;
		}

		float xChange = xPos - lastX;
		float yChange = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
			return;
		}
		xChange *= mouseSensitivity;
		yChange *= mouseSensitivity;

		ProcessMouseMovement(xChange, yChange);
	}

	void ProcessMouseScroll(float yOffset)
	{
		if (FoVy >= 1.0f && FoVy <= 90.0f) {
			FoVy -= yOffset;
		}
		if (FoVy <= 1.0f)
			FoVy = 1.0f;
		if (FoVy >= 90.0f)
			FoVy = 90.0f;
	}

private:
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
	{
		yaw += xOffset;
		pitch += yOffset;

		//std::cout << "yaw = " << yaw << std::endl;
		//std::cout << "pitch = " << pitch << std::endl;

		// Avem grijã sã nu ne dãm peste cap
		if (constrainPitch) {
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		// Se modificã vectorii camerei pe baza unghiurilor Euler
		UpdateCameraVectors();
	}

	void UpdateCameraVectors()
	{
		// Calculate the new forward vector
		this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward.y = sin(glm::radians(pitch));
		this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward = glm::normalize(this->forward);
		// Also re-calculate the Right and Up vector
		right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up = glm::normalize(glm::cross(right, forward));
	}

protected:
	const float cameraSpeedFactor = 200.5f;
	const float mouseSensitivity = 0.1f;

	// Perspective properties
	float zNear;
	float zFar;
	float FoVy;
	int width;
	int height;
	bool isPerspective;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	// Euler Angles
	float yaw;
	float pitch;

	bool bFirstMouseMove = true;
	float lastX = 0.f, lastY = 0.f;
};


class Aquarium
{
public:
	unsigned int VBO1, VAO1, VBO2, VAO2,VBO3,VAO3,VAO4,VBO4;
	unsigned int algaeVAO, algaeVBO;

	Aquarium()
	{
		InitializeBuffers();
	}

	~Aquarium()
	{
		Cleanup();
	}

	void RenderBottom(Shader& shader, const Camera& camera)
	{
		shader.use();
		shader.setMat4("projection", camera.GetProjectionMatrix());
		shader.setMat4("view", camera.GetViewMatrix());

		// Bottom part (solid)
		glm::mat4 model2 = glm::scale(glm::mat4(1.0f), glm::vec3(2.8f, 3.2f, 3.8f));
		model2 = glm::translate(model2, glm::vec3(0.0f, -0.1f, 0.0f));
		shader.setMat4("model", model2);
		shader.SetVec3("objectColor", 0.0f, 0.3f, 0.0f); // Dark green for bottom
		shader.setFloat("transparency", 1.0f);           // Fully opaque
		glBindVertexArray(VAO2);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	void RenderGlass(Shader& shader, const Camera& camera)
	{
		shader.use();
		shader.setMat4("projection", camera.GetProjectionMatrix());
		shader.setMat4("view", camera.GetViewMatrix());

		// Glass part (semi-transparent)
		glm::mat4 model1 = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));
		model1 = glm::translate(model1, glm::vec3(0.0f, -0.15f, 0.0f));

		shader.setMat4("model", model1);
		shader.SetVec3("objectColor", 0.9f, 0.9f, 0.9f); // Light gray for glass
		shader.setFloat("transparency", 0.3f);          // Semi-transparent glass
		glBindVertexArray(VAO1);
		glDrawArrays(GL_TRIANGLES, 0, 30);
	}

	

	void RenderWater(Shader& shader, const Camera& camera, float time, const std::string& texturePath)
	{
		// Load the texture dynamically from the provided path
		unsigned int textureID = LoadTexture(texturePath);

		shader.use();
		shader.setMat4("projection", camera.GetProjectionMatrix());
		shader.setMat4("view", camera.GetViewMatrix());

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f)); // Slightly raise the water
		model = glm::scale(model, glm::vec3(2.97f, 2.6f, 2.97f));   // Make it smaller than the glass

		shader.setMat4("model", model);
		shader.SetVec3("objectColor", 0.0f, 0.5f, 1.0f); // Light blue water
		shader.setFloat("time", time);                   // Pass the time for animation

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);         // Bind the loaded texture
		shader.setInt("texture1", 0);                    // Use texture unit 0

		glBindVertexArray(VAO3);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// Clean up texture after rendering (optional, prevents memory leaks)
		glDeleteTextures(1, &textureID);
	}

	void RenderCuboid(Shader& shader, const Camera& camera)
	{
		shader.use();
		shader.setMat4("projection", camera.GetProjectionMatrix());
		shader.setMat4("view", camera.GetViewMatrix());

		// Set the hardcoded color for the cuboid
		glm::vec3 cuboidColor(0.96f, 0.87f, 0.70f); // Light brown color
		shader.SetVec3("objectColor", cuboidColor.x, cuboidColor.y, cuboidColor.z);

		// Model transformations: translate and scale the cuboid
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Position the cuboid
		model = glm::scale(model, glm::vec3(2.5f, 1.54f, 3.0f));     // Scale the cuboid
		shader.setMat4("model", model);

		// Bind the VAO and draw the cuboid
		glBindVertexArray(VAO4); // Assuming VAO4 is initialized for the cuboid
		glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices for a cube
		glBindVertexArray(0);
	}
	
	void RenderAlgae(Shader& shader, const Camera& camera, float time) {
    shader.use();
    shader.setMat4("projection", camera.GetProjectionMatrix());
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setFloat("time", time);

    glm::vec3 algaeBasePosition(0.0f, -1.78f, 0.0f); // Place algae at the bottom
    int numAlgae = 10; // Number of algae in the bouquet
    float swaySpeed = 2.0f; // Speed of sway animation
    float swayAmplitude = glm::radians(5.0f); // Amplitude of sway
    float spreadAngle = glm::radians(360.0f / numAlgae); // Angle between algae

    for (int i = 0; i < numAlgae; ++i) {
        // Compute rotation angle for each algae
        float angle = i * spreadAngle;

        // Sway animation for natural motion
        float sway = swayAmplitude * glm::sin(time * swaySpeed + i);

        // Transformations
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, algaeBasePosition); // Start at the bottom
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
        model = glm::rotate(model, glm::radians(30.0f) + sway, glm::vec3(1.0f, 0.0f, 0.0f)); // Tilt outward
        model = glm::scale(model, glm::vec3(0.1f, 1.0f, 0.1f)); // Scale algae

        // Set shader uniforms
        shader.setMat4("model", model);
        shader.SetVec3("objectColor", 0.0f, 0.8f, 0.2f); // Green algae
        shader.SetVec3("viewPos", camera.GetPosition());
        shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);

        // Draw algae
        glBindVertexArray(algaeVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, (20 + 1) * 2);
        glBindVertexArray(0);
    }
}







	void InitializeAlgaeBuffer() {
		const int slices = 20; // Number of slices around the cylinder
		const float radius = 0.1f; // Radius of the cylinder
		const float height = 1.0f; // Height of the cylinder

		std::vector<float> vertices;

		// Generate vertices for the cylinder
		for (int i = 0; i <= slices; ++i) {
			float angle = i * 2.0f * M_PI / slices;
			float x = radius * cos(angle);
			float z = radius * sin(angle);

			// Bottom cap
			vertices.push_back(x);  // Position
			vertices.push_back(0.0f);
			vertices.push_back(z);
			vertices.push_back(0.0f); // Normal
			vertices.push_back(-1.0f);
			vertices.push_back(0.0f);

			// Top cap
			vertices.push_back(x);  // Position
			vertices.push_back(height);
			vertices.push_back(z);
			vertices.push_back(0.0f); // Normal
			vertices.push_back(1.0f);
			vertices.push_back(0.0f);
		}

		// Generate and bind VAO and VBO
		glGenVertexArrays(1, &algaeVAO);
		glGenBuffers(1, &algaeVBO);

		glBindVertexArray(algaeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, algaeVBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); // Unbind VAO
	}

private:
	
	void InitializeBuffers()
	{
		// First cube (inner part)
		float vertices1[] = {
			// Fața din față
		-1.0f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 1.0f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 1.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 1.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-1.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-1.0f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		// Fața din spate
		-1.0f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 1.0f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 1.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 1.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-1.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-1.0f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		// Fața stângă
		-1.0f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-1.0f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-1.0f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-1.0f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-1.0f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-1.0f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		// Fața dreaptă
		 1.0f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 1.0f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 1.0f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 1.0f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 1.0f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 1.0f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		 // Fața de jos
		 -1.0f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		  1.0f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		  1.0f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		  1.0f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 -1.0f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 -1.0f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		 -1.0f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		  1.0f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		  1.0f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		  1.0f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 -1.0f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	     -1.0f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f

		};

		glGenVertexArrays(1, &VAO1);
		glGenBuffers(1, &VBO1);

		glBindVertexArray(VAO1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// Second cube (bottom part)
		// Second cube (bottom part)
		// Second cube (bottom part)
		float vertices2[] = {
			// Front face
			-1.2f, -0.501f, -0.5f,  0.0f,  0.0f, -1.0f,
			 1.2f, -0.501f, -0.5f,  0.0f,  0.0f, -1.0f,
			 1.2f, -0.6f, -0.5f,    0.0f,  0.0f, -1.0f,
			 1.2f, -0.6f, -0.5f,    0.0f,  0.0f, -1.0f,
			-1.2f, -0.6f, -0.5f,    0.0f,  0.0f, -1.0f,
			-1.2f, -0.501f, -0.5f,  0.0f,  0.0f, -1.0f,

			// Back face
			-1.2f, -0.501f,  0.5f,  0.0f,  0.0f,  1.0f,
			 1.2f, -0.501f,  0.5f,  0.0f,  0.0f,  1.0f,
			 1.2f, -0.6f,  0.5f,    0.0f,  0.0f,  1.0f,
			 1.2f, -0.6f,  0.5f,    0.0f,  0.0f,  1.0f,
			-1.2f, -0.6f,  0.5f,    0.0f,  0.0f,  1.0f,
			-1.2f, -0.501f,  0.5f,  0.0f,  0.0f,  1.0f,

			// Left face
			-1.2f, -0.6f,  0.5f,  -1.0f,  0.0f,  0.0f,
			-1.2f, -0.6f, -0.5f,  -1.0f,  0.0f,  0.0f,
			-1.2f, -0.501f, -0.5f, -1.0f,  0.0f,  0.0f,
			-1.2f, -0.501f, -0.5f, -1.0f,  0.0f,  0.0f,
			-1.2f, -0.501f,  0.5f, -1.0f,  0.0f,  0.0f,
			-1.2f, -0.6f,  0.5f,  -1.0f,  0.0f,  0.0f,

			// Right face
			 1.2f, -0.6f,  0.5f,   1.0f,  0.0f,  0.0f,
			 1.2f, -0.6f, -0.5f,   1.0f,  0.0f,  0.0f,
			 1.2f, -0.501f, -0.5f, 1.0f,  0.0f,  0.0f,
			 1.2f, -0.501f, -0.5f, 1.0f,  0.0f,  0.0f,
			 1.2f, -0.501f,  0.5f, 1.0f,  0.0f,  0.0f,
			 1.2f, -0.6f,  0.5f,   1.0f,  0.0f,  0.0f,

			 // Bottom face
			 -1.2f, -0.6f, -0.5f,  0.0f, -1.0f,  0.0f,
			  1.2f, -0.6f, -0.5f,  0.0f, -1.0f,  0.0f,
			  1.2f, -0.6f,  0.5f,  0.0f, -1.0f,  0.0f,
			  1.2f, -0.6f,  0.5f,  0.0f, -1.0f,  0.0f,
			 -1.2f, -0.6f,  0.5f,  0.0f, -1.0f,  0.0f,
			 -1.2f, -0.6f, -0.5f,  0.0f, -1.0f,  0.0f,

			 // Top face (newly added)
		 -1.2f, -0.501f, -0.5f,  0.0f,  1.0f,  0.0f,  // Top-left back
		  1.2f, -0.501f, -0.5f,  0.0f,  1.0f,  0.0f,  // Top-right back
		  1.2f, -0.501f,  0.5f,  0.0f,  1.0f,  0.0f,  // Top-right front
		  1.2f, -0.501f,  0.5f,  0.0f,  1.0f,  0.0f,  // Top-right front
		 -1.2f, -0.501f,  0.5f,  0.0f,  1.0f,  0.0f,  // Top-left front
		-1.2f, -0.501f, -0.5f,  0.0f,  1.0f,  0.0f   // Top-left back

		};

		glGenVertexArrays(1, &VAO2);
		glGenBuffers(1, &VBO2);

		glBindVertexArray(VAO2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	void InitializeWaterBuffer() {
		float waterVertices[] = {
			// Positions             // Normals
			// Front face
			-1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,

			// Back face
			-1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,

			// Left face
			-1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,
			-1.0f,  1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,
			-1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,
			-1.0f, -1.0f, -1.0f,    -1.0f,  0.0f,  0.0f,
			-1.0f, -1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,
			-1.0f,  1.0f,  1.0f,    -1.0f,  0.0f,  0.0f,

			// Right face
			 1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,
			 1.0f,  1.0f, -1.0f,     1.0f,  0.0f,  0.0f,
			 1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,
			 1.0f, -1.0f, -1.0f,     1.0f,  0.0f,  0.0f,
			 1.0f, -1.0f,  1.0f,     1.0f,  0.0f,  0.0f,
			 1.0f,  1.0f,  1.0f,     1.0f,  0.0f,  0.0f,

			 // Top face
			 -1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,
			  1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,
			  1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,
			  1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,
			 -1.0f,  1.0f, -1.0f,     0.0f,  1.0f,  0.0f,
			 -1.0f,  1.0f,  1.0f,     0.0f,  1.0f,  0.0f,

			 // Bottom face
			 -1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,
			  1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,
			  1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,
			  1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,
			 -1.0f, -1.0f, -1.0f,     0.0f, -1.0f,  0.0f,
			 -1.0f, -1.0f,  1.0f,     0.0f, -1.0f,  0.0f,
		};

		glGenVertexArrays(1, &VAO3);
		glGenBuffers(1, &VBO3);

		glBindVertexArray(VAO3);
		glBindBuffer(GL_ARRAY_BUFFER, VBO3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}


	void InitializesandBuffers()
	{
		float lightBrownCuboidVertices[] = {
			// Positions             // Normals
			// Bottom Face
			-0.5f, -0.7f, -0.5f,     0.0f, -1.0f,  0.0f,
			 0.5f, -0.7f, -0.5f,     0.0f, -1.0f,  0.0f,
			 0.5f, -0.7f,  0.5f,     0.0f, -1.0f,  0.0f,
			 0.5f, -0.7f,  0.5f,     0.0f, -1.0f,  0.0f,
			-0.5f, -0.7f,  0.5f,     0.0f, -1.0f,  0.0f,
			-0.5f, -0.7f, -0.5f,     0.0f, -1.0f,  0.0f,

			// Top Face
			-0.5f, -0.6f, -0.5f,     0.0f,  1.0f,  0.0f,
			 0.5f, -0.6f, -0.5f,     0.0f,  1.0f,  0.0f,
			 0.5f, -0.6f,  0.5f,     0.0f,  1.0f,  0.0f,
			 0.5f, -0.6f,  0.5f,     0.0f,  1.0f,  0.0f,
			-0.5f, -0.6f,  0.5f,     0.0f,  1.0f,  0.0f,
			-0.5f, -0.6f, -0.5f,     0.0f,  1.0f,  0.0f,

			// Front Face
			-0.5f, -0.7f,  0.5f,     0.0f,  0.0f,  1.0f,
			 0.5f, -0.7f,  0.5f,     0.0f,  0.0f,  1.0f,
			 0.5f, -0.6f,  0.5f,     0.0f,  0.0f,  1.0f,
			 0.5f, -0.6f,  0.5f,     0.0f,  0.0f,  1.0f,
			-0.5f, -0.6f,  0.5f,     0.0f,  0.0f,  1.0f,
			-0.5f, -0.7f,  0.5f,     0.0f,  0.0f,  1.0f,

			// Back Face
			-0.5f, -0.7f, -0.5f,     0.0f,  0.0f, -1.0f,
			 0.5f, -0.7f, -0.5f,     0.0f,  0.0f, -1.0f,
			 0.5f, -0.6f, -0.5f,     0.0f,  0.0f, -1.0f,
			 0.5f, -0.6f, -0.5f,     0.0f,  0.0f, -1.0f,
			-0.5f, -0.6f, -0.5f,     0.0f,  0.0f, -1.0f,
			-0.5f, -0.7f, -0.5f,     0.0f,  0.0f, -1.0f,

			// Left Face
			-0.5f, -0.7f, -0.5f,    -1.0f,  0.0f,  0.0f,
			-0.5f, -0.7f,  0.5f,    -1.0f,  0.0f,  0.0f,
			-0.5f, -0.6f,  0.5f,    -1.0f,  0.0f,  0.0f,
			-0.5f, -0.6f,  0.5f,    -1.0f,  0.0f,  0.0f,
			-0.5f, -0.6f, -0.5f,    -1.0f,  0.0f,  0.0f,
			-0.5f, -0.7f, -0.5f,    -1.0f,  0.0f,  0.0f,

			// Right Face
			 0.5f, -0.7f, -0.5f,     1.0f,  0.0f,  0.0f,
			 0.5f, -0.7f,  0.5f,     1.0f,  0.0f,  0.0f,
			 0.5f, -0.6f,  0.5f,     1.0f,  0.0f,  0.0f,
			 0.5f, -0.6f,  0.5f,     1.0f,  0.0f,  0.0f,
			 0.5f, -0.6f, -0.5f,     1.0f,  0.0f,  0.0f,
			 0.5f, -0.7f, -0.5f,     1.0f,  0.0f,  0.0f,
		};

		glGenVertexArrays(1, &VAO4);
		glGenBuffers(1, &VBO4);
		

		// Bind VAO and VBO
		glBindVertexArray(VAO4);
		glBindBuffer(GL_ARRAY_BUFFER, VBO4);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lightBrownCuboidVertices), lightBrownCuboidVertices, GL_STATIC_DRAW);

		// Set vertex attribute pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Texture Coords
		glEnableVertexAttribArray(2);

		// Unbind VAO
		glBindVertexArray(0);
	}
	
	void Cleanup()
	{
		glDeleteVertexArrays(1, &VAO1);
		glDeleteBuffers(1, &VBO1);
		glDeleteVertexArrays(1, &VAO2);
		glDeleteBuffers(1, &VBO2);
		glDeleteVertexArrays(1, &VAO3);
		glDeleteBuffers(1, &VBO3);
	}

};


GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;
float g_fKA = 0.5, g_fKD = 0.5f, g_fKS = 0.5;
int g_fN = 2;

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		g_fKA += 0.1f;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		g_fKA -= 0.1f;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		g_fKD += 0.1f;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		g_fKD -= 0.1f;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		g_fKS += 0.1f;
	}
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		g_fKS -= 0.1f;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		g_fN *= 2;
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		g_fN /= 2;
		if (g_fN < 1)
		{
			g_fN = 2;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);

	}
}
void OpenGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::cerr << "OpenGL Debug Message: " << message << std::endl;
}



int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab 7", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	// Lock the cursor to the window and disable its visibility
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	



	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 0.0, 6.0));
	glm::vec3 lightPos(0.0f, 0.0f, 1.0f);


	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	Shader lightingShader("PhongLight.vs", "PhongLight.fs");
	Shader lampShader("Lamp.vs", "Lamp.fs");
	Shader waterShader("Water.vs", "Water.fs");
	Shader algaeShader("Algae.vs", "Algae.fs");
	Shader lightingWithTextureShader("PhongLightWithTexture.vs", "PhongLightWithTexture.fs");\
	Aquarium aquarium;


	std::string piratObjFileName = (currentPath + "\\Models\\Fish\\Fish.obj");
	Model piratObjModel(piratObjFileName, false);

	std::string texturePath = (currentPath + "\\Models\\water.jpg");
	

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.68f, 0.85f, 0.90f, 1.0f); // RGB pentru albastru deschis

		

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float lightSpeed = currentFrame * 100.f;
		float lightRadius = 5.f;
		lightPos.x = lightRadius * glm::sin(glm::radians(lightSpeed));
		lightPos.z = lightRadius * glm::cos(glm::radians(lightSpeed));

		lightingShader.use();
		lightingShader.setFloat("transparency", 0.3f);
		lightingShader.SetVec3("objectColor", 0.7f, 0.7f, 0.7f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingShader.setFloat("KA", g_fKA);
		lightingShader.setFloat("KD", g_fKD);
		lightingShader.setFloat("KS", g_fKS);

		// Render opaque objects first (e.g., bottom)

		

		glDepthMask(GL_TRUE);
		glDisable(GL_CULL_FACE);
		aquarium.RenderBottom(lightingShader, *pCamera);
		aquarium.RenderCuboid(lightingShader, *pCamera);
		aquarium.InitializeAlgaeBuffer();
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame);
		glDepthMask(GL_FALSE);
		aquarium.RenderGlass(lightingShader, *pCamera);
		aquarium.RenderWater(waterShader, *pCamera, glfwGetTime(), texturePath);
		glDepthMask(GL_TRUE);

		
		




		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingWithTextureShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingWithTextureShader.SetVec3("lightPos", lightPos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);


		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());

		//glm::mat4 piratModel = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.8f, 0.0f)); // Ridică peștele
		//piratModel = glm::scale(piratModel, glm::vec3(5.0f)); // Ajustează dimensiunea peștelui

		////////////////////////////Translatie peste /////////////////////////////////////////////////
		/*float angle = glm::radians((float)currentFrame * 50.0f); // Crește unghiul în funcție de timp
		float radius = 2.0f; // Raza cercului

		//// Translație pe cerc
		glm::mat4 piratModel = glm::translate(glm::mat4(1.0f), glm::vec3(
			radius * glm::cos(angle),  // x
			0.8f,                      // y (înălțimea fixă)
			radius * glm::sin(angle)   // z
		));

		//// Rotație în jurul propriei axe
		piratModel = glm::rotate(piratModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		//Ajustare dimensiune
		piratModel = glm::scale(piratModel, glm::vec3(5.0f));*/

		float angleFish1 = glm::radians((float)currentFrame * 50.0f); // Unghi în funcție de timp
		float radiusFish1 = 2.0f; // Raza cercului pentru primul pește

		// Poziția peștelui pe cerc
		glm::vec3 positionFish1(
			radiusFish1 * glm::cos(angleFish1),  // x
			0.2f,                                // y (înălțimea fixă)
			radiusFish1 * glm::sin(angleFish1)   // z
		);

		// Direcția tangentială pentru orientare
		glm::vec3 directionFish1(
			-glm::sin(angleFish1),  // x (tangenta pe x)
			0.0f,                   // y (nu se mișcă pe verticală)
			glm::cos(angleFish1)    // z (tangenta pe z)
		);

		// Construim matricea pentru pește
		glm::mat4 fish1Model = glm::mat4(1.0f);
		fish1Model = glm::translate(fish1Model, positionFish1); // Translație
		fish1Model = glm::rotate(fish1Model, glm::atan(directionFish1.x, directionFish1.z), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotește peștele după direcție
		fish1Model = glm::scale(fish1Model, glm::vec3(5.0f)); // Scalare

		// Desenează primul pește
		lightingWithTextureShader.setMat4("model", fish1Model);
		piratObjModel.Draw(lightingWithTextureShader);



		// Actualizează poziția peștelui
		linearFishPosition += linearFishDirection * (float)(deltaTime * 1.0f); // Viteză constantă

		// Verifică coliziunea cu marginile acvariului
		if (!isSinusoidal &&
			(linearFishPosition.x > aquariumBoundary || linearFishPosition.x < -aquariumBoundary)) {
			linearFishDirection.x = -linearFishDirection.x; // Inversează direcția pe x
			isSinusoidal = true; // Începe rotația sinusoidală
			sinusoidalTime = 0.0f; // Resetează timpul sinusoidal
		}
		if (!isSinusoidal &&
			(linearFishPosition.z > aquariumBoundary || linearFishPosition.z < -aquariumBoundary)) {
			linearFishDirection.z = -linearFishDirection.z; // Inversează direcția pe z
			isSinusoidal = true; // Începe rotația sinusoidală
			sinusoidalTime = 0.0f; // Resetează timpul sinusoidal
		}

		// Aplica rotația sinusoidală dacă este activă
		glm::mat4 rotation = glm::mat4(1.0f); // Identitate
		if (isSinusoidal) {
			sinusoidalTime += (float)(deltaTime * 2.0f); // Crește timpul sinusoidal

			// Aplica rotația sinusoidală
			float angle = glm::sin(sinusoidalTime * glm::pi<float>()) * glm::radians(45.0f); // Amplitudine de 45°
			rotation = glm::rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotație pe axa Y

			// Dacă rotația este completă, oprește sinusoidalul
			if (sinusoidalTime >= 1.0f) {
				isSinusoidal = false; // Oprește rotația sinusoidală
			}
		}

		// Creează modelul peștelui pentru această mișcare
		glm::mat4 linearFishModel = glm::translate(glm::mat4(1.0f), linearFishPosition); // Translație
		linearFishModel *= glm::lookAt(
			glm::vec3(0.0f),                // Poziția peștelui (translația e separată)
			linearFishDirection,            // Direcția de mișcare
			glm::vec3(0.0f, 1.0f, 0.0f)     // Vectorul „up”
		);

		// Adaugă rotația sinusoidală
		linearFishModel *= rotation;

		// Ajustează dimensiunea
		linearFishModel = glm::scale(linearFishModel, glm::vec3(5.0f));

		// Setează matricea în shader și desenează peștele
		lightingWithTextureShader.setMat4("model", linearFishModel);
		piratObjModel.Draw(lightingWithTextureShader);





		// Desenează acvariul transparent


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();


	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}