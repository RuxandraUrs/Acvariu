#include <Windows.h>
#include <locale>
#include <codecvt>
#include <cmath>


#include <GL/glew.h>

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>



#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "Aquarium.h"
#include "Skybox.h"

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

unsigned int LoadCubemap(std::vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void CheckBoundaryCollision(glm::vec3& position, glm::vec3& direction, float boundary) {
	if (position.x > boundary || position.x < -boundary) {
		direction.x = -direction.x;
		position.x = glm::clamp(position.x, -boundary, boundary);
	}
	if (position.z > boundary || position.z < -boundary) {
		direction.z = -direction.z;
		position.z = glm::clamp(position.z, -boundary, boundary);
	}
}

bool IsColliding(const glm::vec3& pos1, const glm::vec3& pos2, float minDistance) {
	return glm::distance(pos1, pos2) < minDistance;
}

void ResolveCollision(glm::vec3& pos1, glm::vec3& pos2, glm::vec3& dir1, glm::vec3& dir2) {
	glm::vec3 displacement = glm::normalize(pos1 - pos2) * 0.05f; // Push fishes apart slightly
	pos1 += displacement;
	pos2 -= displacement;
	dir1 = glm::reflect(dir1, glm::normalize(displacement));
	dir2 = glm::reflect(dir2, glm::normalize(-displacement));
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
	Shader skyboxShader("Skybox.vs", "Skybox.fs");
	Shader lightingWithTextureShader("PhongLightWithTexture.vs", "PhongLightWithTexture.fs"); \
		Aquarium aquarium;


	std::string piratObjFileName = (currentPath + "\\Models\\Fish\\Fish.obj");
	Model piratObjModel(piratObjFileName, false);

	std::string grassPath = (currentPath + "\\Models\\Grass\\grass.obj");
	Model grassModel(grassPath, false);

	// Add this block for generating transformation matrices
	std::vector<glm::mat4> grassTransforms;

	// Generate transformations for the grass patches
	const float gridSize = 5.0f; // Floor size
	const float grassSize = 1.0f; // Size of a single patch
	const int gridCount = gridSize / grassSize;

	for (int x = 0; x < gridCount; ++x) {
		for (int z = 0; z < gridCount; ++z) {
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(
				-gridSize / 2.0f + x * grassSize,
				-1.0f,
				-gridSize / 2.0f + z * grassSize
			));
			transform = glm::scale(transform, glm::vec3(0.5f));
			grassTransforms.push_back(transform);
		}
	}

	// Create instance buffer
	unsigned int instanceBuffer;
	glGenBuffers(1, &instanceBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
	glBufferData(GL_ARRAY_BUFFER, grassTransforms.size() * sizeof(glm::mat4), &grassTransforms[0], GL_STATIC_DRAW);

	// Link instance buffer to grass model's VAO
	for (unsigned int i = 0; i < grassModel.meshes.size(); i++) {
		glBindVertexArray(grassModel.meshes[i].VAO);

		GLsizei vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

	std::string texturePath = (currentPath + "\\Models\\water.jpg");
	std::vector<std::string> faces = {
	currentPath+"\\Models\\Daylight Box_Right.bmp",
	currentPath + "\\Models\\Daylight Box_Left.bmp",
	currentPath + "\\Models\\Daylight Box_Top.bmp",
	currentPath + "\\Models\\Daylight Box_Bottom.bmp",
	currentPath + "\\Models\\Daylight Box_Front.bmp",
	currentPath+"\\Models\\Daylight Box_Back.bmp"
	};
	Skybox skybox(faces);
	

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
		

		glDepthFunc(GL_LEQUAL); // Change depth function to allow skybox depth
		skyboxShader.use();

		// Pass the projection and modified view matrix to the skybox shader
		glm::mat4 viewMatrix = glm::mat4(glm::mat3(pCamera->GetViewMatrix())); // Remove translation
		skyboxShader.setMat4("view", viewMatrix);
		skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());

		// Render the skybox
		skybox.Render(skyboxShader, pCamera->GetViewMatrix(), pCamera->GetProjectionMatrix());

		glDepthFunc(GL_LESS);

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
		glm::mat4 grassModelMatrix1 = glm::mat4(1.0f);
		grassModelMatrix1 = glm::translate(grassModelMatrix1, glm::vec3(-0.5f, -1.8f, 0.0f)); // Position of the first grass
		grassModelMatrix1 = glm::scale(grassModelMatrix1, glm::vec3(6.5f, 6.0f, 7.0f));       // Scale of the first grass

		lightingWithTextureShader.use();
		lightingWithTextureShader.setMat4("model", grassModelMatrix1);
		grassModel.Draw(lightingWithTextureShader);

		// Render the second grass object
		glm::mat4 grassModelMatrix2 = glm::mat4(1.0f);
		grassModelMatrix2 = glm::translate(grassModelMatrix2, glm::vec3(1.7f, -1.8f, 0.0f)); // Position of the second grass
		grassModelMatrix2 = glm::scale(grassModelMatrix2, glm::vec3(6.0f, 6.0f, 6.0f));      // Scale of the second grass

		lightingWithTextureShader.use();
		lightingWithTextureShader.setMat4("model", grassModelMatrix2);
		grassModel.Draw(lightingWithTextureShader);

		glm::mat4 grassModelMatrix3 = glm::mat4(1.0f);
		grassModelMatrix3 = glm::translate(grassModelMatrix3, glm::vec3(1.7f, -1.8f, 0.43f)); // Position of the second grass
		grassModelMatrix3 = glm::scale(grassModelMatrix3, glm::vec3(5.5f, 5.0f, 5.0f));      // Scale of the second grass

		lightingWithTextureShader.use();
		lightingWithTextureShader.setMat4("model", grassModelMatrix3);
		grassModel.Draw(lightingWithTextureShader);

		glDepthMask(GL_TRUE);
		glDisable(GL_CULL_FACE);
		aquarium.RenderBottom(lightingShader, *pCamera);
		aquarium.RenderCuboid(lightingShader, *pCamera);
		aquarium.InitializeAlgaeBuffer();
		// Render 33 algae inside the aquarium, including the sides

// Back and front edges (8 algae)
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-1.2f, -1.8f, -1.0f), 10, glm::vec3(0.0f, 0.8f, 0.2f)); // Back-left edge
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(1.2f, -1.8f, -1.0f), 10, glm::vec3(0.8f, 0.2f, 0.2f));  // Back-right edge
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-1.2f, -1.8f, 1.0f), 10, glm::vec3(0.2f, 0.8f, 0.8f));  // Front-left edge
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(1.2f, -1.8f, 1.0f), 10, glm::vec3(0.6f, 0.7f, 0.2f));   // Front-right edge

		// Center gaps (6 algae)
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(0.0f, -1.8f, -0.8f), 10, glm::vec3(0.3f, 0.5f, 0.8f));  // Center back
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(0.0f, -1.8f, 0.8f), 10, glm::vec3(0.8f, 0.3f, 0.5f));   // Center front
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-0.8f, -1.8f, 0.0f), 10, glm::vec3(0.3f, 0.7f, 0.2f));  // Center-left
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(0.8f, -1.8f, 0.0f), 10, glm::vec3(0.2f, 0.5f, 0.8f));   // Center-right

		// Near glass sides (8 algae, at -2.8f and 2.8f on X)
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-2.8f, -1.8f, -1.0f), 10, glm::vec3(0.6f, 0.4f, 0.7f)); // Side left-back
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-2.8f, -1.8f, 1.0f), 10, glm::vec3(0.7f, 0.3f, 0.6f));  // Side left-front
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(2.8f, -1.8f, -1.0f), 10, glm::vec3(0.3f, 0.6f, 0.7f));  // Side right-back
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(2.8f, -1.8f, 1.0f), 10, glm::vec3(0.8f, 0.3f, 0.4f));   // Side right-front
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-2.8f, -1.8f, -0.5f), 10, glm::vec3(0.5f, 0.2f, 0.8f)); // Side left-center
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(2.8f, -1.8f, -0.5f), 10, glm::vec3(0.6f, 0.3f, 0.7f));  // Side right-center
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-2.8f, -1.8f, 0.5f), 10, glm::vec3(0.3f, 0.7f, 0.5f));  // Side left-center front
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(2.8f, -1.8f, 0.5f), 10, glm::vec3(0.2f, 0.5f, 0.8f));   // Side right-center front

		// Diagonal spaces (6 algae)
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-1.6f, -1.8f, -0.8f), 10, glm::vec3(0.8f, 0.6f, 0.4f)); // Back diagonal left
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(1.6f, -1.8f, 0.8f), 10, glm::vec3(0.3f, 0.6f, 0.7f));   // Front diagonal right
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-1.6f, -1.8f, 0.8f), 10, glm::vec3(0.6f, 0.7f, 0.3f));  // Front diagonal left
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(1.6f, -1.8f, -0.8f), 10, glm::vec3(0.4f, 0.8f, 0.6f));  // Back diagonal right

		// Fill empty mid-ground spaces (5 algae)
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-1.0f, -1.8f, -0.3f), 10, glm::vec3(0.5f, 0.3f, 0.8f));
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(1.0f, -1.8f, 0.3f), 10, glm::vec3(0.3f, 0.8f, 0.6f));
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(0.3f, -1.8f, 0.0f), 10, glm::vec3(0.8f, 0.5f, 0.3f));
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(-0.3f, -1.8f, -0.3f), 10, glm::vec3(0.6f, 0.4f, 0.7f));
		aquarium.RenderAlgae(algaeShader, *pCamera, currentFrame, glm::vec3(0.0f, -1.8f, 0.0f), 10, glm::vec3(0.4f, 0.6f, 0.8f));

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

		// Fish parameters
		// Fish parameters
		// Fish parameters
		// Fish movement parameters
		// Fish parameters for elliptical movement


		const int fishCount = 2; // Number of fishes moving in ellipses
		float fishSpeeds[fishCount] = { 0.5f, 0.3f }; // Speed of rotation for each fish
		float fishRadiiX[fishCount] = { 2.0f, 1.5f }; // Horizontal radius (a)
		float fishRadiiZ[fishCount] = { 1.0f, 0.8f }; // Vertical radius (b)
		float fishHeights[fishCount] = { 0.0f, 0.3f }; // Heights for each fish

		// Fish rendering loop
		for (int i = 0; i < fishCount; i++) {
			// Compute the angle based on time and speed
			float angle = (float)glfwGetTime() * fishSpeeds[i];

			// Calculate position using ellipse equations
			float x = fishRadiiX[i] * cos(angle);
			float z = fishRadiiZ[i] * sin(angle);
			glm::vec3 fishPosition(x, fishHeights[i], z);

			// Tangent direction for rotation
			glm::vec3 fishDirection(-sin(angle), 0.0f, cos(angle));
			float fishAngle = glm::atan(fishDirection.x, fishDirection.z);

			// Apply transformations: translate, rotate, scale
			glm::mat4 fishModel = glm::mat4(1.0f);
			fishModel = glm::translate(fishModel, fishPosition);
			fishModel = glm::rotate(fishModel, fishAngle, glm::vec3(0.0f, 1.0f, 0.0f));
			fishModel = glm::scale(fishModel, glm::vec3(5.0f)); // Scale fish size

			// Set shader uniforms and render fish model
			lightingWithTextureShader.setMat4("model", fishModel);
			piratObjModel.Draw(lightingWithTextureShader);
		}



		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();



	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

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