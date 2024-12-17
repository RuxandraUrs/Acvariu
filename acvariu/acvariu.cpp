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