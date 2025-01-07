#include <stb_image.h>

#include <Windows.h>
#include <locale>
#include <codecvt>
#include <thread> 
#include <chrono> 

#include <stdlib.h> 
#include <stdio.h>
#include <math.h> 
#include <map>

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Aquarium.h"
#include "Skybox.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;




Camera* pCamera = nullptr;

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


double deltaTime = 0.0f;	
double lastFrame = 0.0f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static float rotationAngle = 0.0f; // Unghiul curent de rotație
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		rotationAngle += 1.0f; // Crește unghiul de rotație
		float distanceFromCenter = 20.0f; // Distanța camerei față de centru
		glm::vec3 aquariumCenter(10.0f, 5.0f, 10.0f); // Centru ipotetic al acvariului
		pCamera->RotateAroundPoint(aquariumCenter, rotationAngle, distanceFromCenter);
	}


	// Restul logicii existente
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
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);
}


std::string getCurentPath() {
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);
	return currentPath;
}

unsigned int CreateTexture(const std::string& strTexturePath)
{
	unsigned int textureId = -1;

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(strTexturePath.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
	else {
		std::cout << "Failed to load texture: " << strTexturePath << std::endl;
	}
	stbi_image_free(data);

	return textureId;
}

std::string currentPath = getCurentPath();



//declare model
Model fishObjModel;
Model coralObjModel;
Model fish2ObjModel;
Model treasureObjModel;
Model fish5ObjModel;
Model fish8ObjModel;
Model GrassModel;
Model ChestModel;
Model ShipModel;
Model FishModel;
Model CoralModel;

float incrementSpeed = 0.04;
float incrementRotation = 2.4;

static float elapsedTime = 0.0f;
float xAxis = 5.0f; 
float zAxis = 3.0f; 
float anglespeed = 0.2f;

void renderScene(Shader& shader, Aquarium& aquarium, Camera& camera) {




	shader.setBool("useTexture", false);
	shader.SetVec3("objectColor", 0.0f, 0.0f, 1.0f); 
	shader.setFloat("transparency", 1.0f); 
	aquarium.RenderBottom(shader, camera);
	shader.setBool("useTexture", true);

	aquarium.RenderCuboid(shader, camera);


	glm::mat4 model{ glm::mat4(1.0f) };

	fish2ObjModel.move(incrementSpeed, incrementRotation);
	model = glm::translate(glm::mat4(1.0f), fish2ObjModel.currentPos);
	model = glm::scale(model, glm::vec3(0.1f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(fish2ObjModel.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	shader.setMat4("model", model);
	fish2ObjModel.Draw(shader);

	fish5ObjModel.move(incrementSpeed, incrementRotation);
	model = glm::translate(glm::mat4(1.0f), fish5ObjModel.currentPos);
	model = glm::scale(model, glm::vec3(0.1f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(fish5ObjModel.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	shader.setMat4("model", model);
	fish5ObjModel.Draw(shader);



	fish8ObjModel.move(incrementSpeed, incrementRotation);
	model = glm::translate(glm::mat4(1.0f), fish8ObjModel.currentPos);
	model = glm::scale(model, glm::vec3(0.08f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(fish8ObjModel.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	shader.setMat4("model", model);
	fish8ObjModel.Draw(shader);


	model = glm::translate(glm::mat4(1.0f), glm::vec3(16.8f, -0.2f, 4.6f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	shader.setMat4("model", model);
	ChestModel.Draw(shader);


	model = glm::translate(glm::mat4(1.0f), glm::vec3(17.8f, 1.0f, -1.f));
	model = glm::scale(model, glm::vec3(0.07f));
	shader.setMat4("model", model);
	CoralModel.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(4.8f, 0.2f, 4.6f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	shader.setMat4("model", model);
	ShipModel.Draw(shader);


	elapsedTime += deltaTime * anglespeed;

	// Elliptical path calculations
	float x = xAxis * cos(elapsedTime); // X-coordinate
	float z = zAxis * sin(elapsedTime); // Z-coordinate
	float y = 1.0f; 

	// Position and orientation of the fish
	model = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f + x, y, 2.0f + z));
	model = glm::rotate(model, -elapsedTime, glm::vec3(0.0f, 1.0f, 0.0f)); // Align fish direction
	model = glm::scale(model, glm::vec3(10.1f)); // Adjust size
	shader.setMat4("model", model);
	FishModel.Draw(shader);

	y = 3.0f;
	x = x + 15.0f;
	z = z + 1.0f;
	model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 2.0f + z));
	model = glm::rotate(model, -elapsedTime, glm::vec3(0.0f, 1.0f, 0.0f)); // Align fish direction
	model = glm::scale(model, glm::vec3(17.1f)); // Adjust size
	shader.setMat4("model", model);
	FishModel.Draw(shader);





}



int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Aquarium", NULL, NULL);
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

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	glewInit();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);





	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(10.0f, 3.0f, 10.0f));

	Shader shadowMappingShader((currentPath + "\\Shaders\\PhongLightMix.vs").c_str(), (currentPath + "\\Shaders\\PhongLightMix.fs").c_str());
	Shader shadowMappingDepthShader((currentPath + "\\Shaders\\PhongLightMixDepth.vs").c_str(), (currentPath + "\\Shaders\\PhongLightMixDepth.fs").c_str());
	Shader lampShader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());
	Shader windowShader((currentPath + "\\Shaders\\GlassShader.vs").c_str(), (currentPath + "\\Shaders\\GlassShader.fs").c_str());
	Shader skyboxShader((currentPath + "\\Shaders\\Skybox.vs").c_str(), (currentPath + "\\Shaders\\Skybox.fs").c_str());
	Shader waterShader((currentPath + "\\Shaders\\Water.vs").c_str(), (currentPath + "\\Shaders\\Water.fs").c_str());
	Shader algeeShader((currentPath + "\\Shaders\\Algae.vs").c_str(), (currentPath + "\\Shaders\\Algae.fs").c_str());

	unsigned int floorTexture = CreateTexture((currentPath + "\\Textures\\sand.jpg").c_str());


	std::string grassFileNmae = (currentPath + "\\Models\\Grass\\grass.obj");
	GrassModel = Model(grassFileNmae, false);
	GrassModel.setPos(glm::vec3(2.0f, 1.0f, 3.3f), glm::vec3(12.0f, 2.0f, 2.0f), 180.0f);

	std::string chestFileNmae = (currentPath + "\\Models\\Chest\\chest.obj");
	ChestModel = Model(chestFileNmae, false);
	ChestModel.setPos(glm::vec3(2.0f, 1.0f, 3.3f), glm::vec3(12.0f, 2.0f, 2.0f), 180.0f);

	std::string shipFileNmae = (currentPath + "\\Models\\Ship\\ship.obj");
	ShipModel = Model(shipFileNmae, false);
	ShipModel.setPos(glm::vec3(2.0f, 1.0f, 3.3f), glm::vec3(12.0f, 2.0f, 2.0f), 180.0f);

	std::string coralFileNmae = (currentPath + "\\Models\\Coral3\\model.obj");
	CoralModel = Model(coralFileNmae, false);
	CoralModel.setPos(glm::vec3(2.0f, 1.0f, 3.3f), glm::vec3(12.0f, 2.0f, 2.0f), 180.0f);

	std::string fish2ObjFileName = (currentPath + "\\Models\\Fish2\\bluefish.obj");
	fish2ObjModel = Model(fish2ObjFileName, false);
	fish2ObjModel.setPos(glm::vec3(1.0f, 3.0f, 3.0f), glm::vec3(15.0f, 2.5f, 1.8f), 90.0f);

	std::string fish5ObjFileName = (currentPath + "\\Models\\fish5\\color_fish.obj");
	fish5ObjModel = Model(fish5ObjFileName, false);
	fish5ObjModel.setPos(glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(18.0f, 5.5f, 4.5f), 180.0f);

	std::string FishObjFileName = (currentPath + "\\Models\\Fish\\Fish.obj");
	FishModel = Model(FishObjFileName, false);
	FishModel.setPos(glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(18.0f, 5.5f, 4.5f), 180.0f);

	std::string fish8ObjFileName = (currentPath + "\\Models\\fish8\\black_fish.obj");
	fish8ObjModel = Model(fish8ObjFileName, false);
	fish8ObjModel.setPos(glm::vec3(1.9f, 2.8f, 0.4f), glm::vec3(2.3f, 2.8f, 4.6f), 270.0f);

	std::string WatertexturePath = (currentPath + "\\Textures\\water.jpg");

	//load skybox
	vector<std::string> faces
	{
		currentPath + "\\Textures\\Skybox\\Daylight Box_Right.bmp",
		currentPath + "\\Textures\\Skybox\\Daylight Box_Left.bmp",
		currentPath + "\\Textures\\Skybox\\Daylight Box_Bottom.bmp",
		currentPath + "\\Textures\\Skybox\\Daylight Box_Top.bmp",
		currentPath + "\\Textures\\Skybox\\Daylight Box_Front.bmp",
		currentPath + "\\Textures\\Skybox\\Daylight Box_Back.bmp"
	};

	vector<std::string> facesnight
	{
		currentPath + "\\Textures\\Skybox\\space_rt.png",
		currentPath + "\\Textures\\Skybox\\space_lf.png",
		currentPath + "\\Textures\\Skybox\\space_up.png",
		currentPath + "\\Textures\\Skybox\\space_dn.png",
		currentPath + "\\Textures\\Skybox\\space_ft.png",
		currentPath + "\\Textures\\Skybox\\space_bk.png"
	};
	Skybox skybox(faces);

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glfwSetKeyCallback(window, key_callback);


	shadowMappingShader.use();
	shadowMappingShader.setInt("diffuseTexture", 0);
	shadowMappingShader.setInt("shadowMap", 1);



	// lighting info
   // -------------
	glm::vec3 lightPos(20.0f, 4.0f, 2.5f);

	glEnable(GL_CULL_FACE);




	Aquarium aquarium;

	const double TARGET_FPS = 60.0;
	const double FRAME_DURATION = 1.0 / TARGET_FPS;


	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 20.5f;
		lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);

		

		// render scene from light's point of view
		


		time_t now = time(0);
		tm localTime;
		localtime_s(&localTime, &now);
		int hours = localTime.tm_hour;
		int minutes = localTime.tm_min;
		int seconds = localTime.tm_sec;



		float timeOfDay = hours + (minutes / 60.0f) + (seconds / 3600.0f);

		bool isDaytime = timeOfDay >= 6.0f && timeOfDay < 18.0f;

		float timeNormalized = (hours + (minutes / 60.0f) + (seconds / 3600.0f)) / 24.0f;

		float lightRadius = 22.0f;
		float maxSunHeight = 6.0f;
		float pi = glm::pi<float>();

		float angle = pi * (timeNormalized * 2.0f - 0.5f);


		glm::vec3 sunPosition(
			-lightRadius * cos(angle),
			maxSunHeight* sin(angle),
			2.0f
		);
		float intensity = glm::clamp(sunPosition.y / maxSunHeight, 0.0f, 1.0f);

		// Adjust light color for sunrise/sunset and midday
		glm::vec3 lightColor = glm::mix(
			glm::vec3(0.0f, 0.0f, 0.8f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 
			intensity
		);

		lightView = glm::lookAt(sunPosition, glm::vec3(10.0f, 0.0f, 4.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		shadowMappingDepthShader.use();
		shadowMappingDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		shadowMappingShader.SetVec3("lightPos", sunPosition);
		shadowMappingShader.SetVec3("lightColor", lightColor);
		static bool currentIsDaytime = true;



		if (isDaytime != currentIsDaytime) {
			if (isDaytime) {
				skybox.ReloadTextures(faces); // Load daytime textures
			}
			else {
				skybox.ReloadTextures(facesnight); // Load nighttime textures
			}
			currentIsDaytime = isDaytime;
		}

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		renderScene(shadowMappingDepthShader, aquarium, *pCamera);
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render the scene
		// 2. render scene as normal using the generated depth/shadow map 

		shadowMappingShader.use();
		glm::mat4 projection = pCamera->GetProjectionMatrix();
		glm::mat4 view = pCamera->GetViewMatrix();
		shadowMappingShader.setMat4("projection", projection);
		shadowMappingShader.setMat4("view", view);
		// set light uniforms
		shadowMappingShader.SetVec3("viewPos", pCamera->GetPosition());
		shadowMappingShader.SetVec3("lightPos", sunPosition);
		shadowMappingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDisable(GL_CULL_FACE);
		renderScene(shadowMappingShader, aquarium, *pCamera);
		glm::mat4 model{ glm::mat4(1.0f) };




		model = glm::translate(glm::mat4(1.0f), glm::vec3(7.6f, -0.1f, 1.6f));
		model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f));
		shadowMappingShader.setMat4("model", model);
		GrassModel.Draw(shadowMappingShader);


		model = glm::translate(glm::mat4(1.0f), glm::vec3(14.f, -0.1f, 0.3f));
		model = glm::rotate(model, glm::radians(-200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f));
		shadowMappingShader.setMat4("model", model);
		GrassModel.Draw(shadowMappingShader);


		model = glm::translate(glm::mat4(1.0f), glm::vec3(7.6f, -0.1f, 5.6f));
		model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f));
		shadowMappingShader.setMat4("model", model);
		GrassModel.Draw(shadowMappingShader);


		model = glm::translate(glm::mat4(1.0f), glm::vec3(14.f, -0.1f, 4.5f));
		model = glm::rotate(model, glm::radians(-200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f));
		shadowMappingShader.setMat4("model", model);
		GrassModel.Draw(shadowMappingShader);

		glEnable(GL_BLEND);
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame , glm::vec3(1.5f, 0.2f, -1.8f), 10, glm::vec3(0.2f, 0.8f, 0.1f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(2.3f, 0.2f, 0.5f), 10, glm::vec3(0.7f, 0.3f, 0.2f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(3.8f, 0.2f, 1.2f), 10, glm::vec3(0.3f, 0.6f, 0.8f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(5.1f, 0.2f, 2.7f), 10, glm::vec3(0.9f, 0.2f, 0.5f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(6.4f, 0.2f, -0.9f), 10, glm::vec3(0.1f, 0.5f, 0.7f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(7.2f, 0.2f, 3.6f), 10, glm::vec3(0.4f, 0.9f, 0.3f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(8.5f, 0.2f, 1.8f), 10, glm::vec3(0.8f, 0.4f, 0.6f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(9.9f, 0.2f, 0.0f), 10, glm::vec3(0.5f, 0.7f, 0.2f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(11.3f, 0.2f, -1.5f), 10, glm::vec3(0.3f, 0.5f, 0.9f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(12.6f, 0.2f, 2.4f), 10, glm::vec3(0.7f, 0.2f, 0.4f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(13.8f, 0.2f, 1.0f), 10, glm::vec3(0.2f, 0.6f, 0.8f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(15.0f, 0.2f, -0.7f), 10, glm::vec3(0.8f, 0.3f, 0.5f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(15.2f, 0.2f, 3.2f), 10, glm::vec3(0.6f, 0.9f, 0.2f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(17.5f, 0.2f, 2.0f), 10, glm::vec3(0.9f, 0.1f, 0.7f));
		//aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(18.8f, 0.2f, 0.8f), 10, glm::vec3(0.4f, 0.5f, 0.9f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(19.6f, 0.2f, -1.2f), 10, glm::vec3(0.5f, 0.8f, 0.3f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(2.5f, 0.2f, 6.8f), 10, glm::vec3(0.9f, 0.4f, 0.6f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(4.8f, 0.2f, 7.5f), 10, glm::vec3(0.3f, 0.7f, 0.5f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(6.2f, 0.2f, 6.0f), 10, glm::vec3(0.7f, 0.3f, 0.9f));
		aquarium.RenderAlgae(algeeShader, *pCamera, currentFrame, glm::vec3(8.9f, 0.2f, 5.2f), 10, glm::vec3(0.5f, 0.6f, 0.7f));

		glDisable(GL_BLEND);


		{
		// draw skybox
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		glm::mat4 view = glm::mat4(glm::mat3(pCamera->GetViewMatrix())); // remove translation from the view matrix
		view = glm::scale(view, glm::vec3(-1.0f, -1.0f, -1.0f));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());
		// skybox cube
		skybox.Render(skyboxShader, pCamera->GetViewMatrix(), pCamera->GetProjectionMatrix()); // set depth function back to default
	}

	

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		windowShader.use();
		aquarium.RenderGlass(windowShader, *pCamera);

		glDepthMask(GL_TRUE);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		aquarium.RenderWater(waterShader, *pCamera,currentFrame, WatertexturePath);
		glDepthMask(GL_TRUE);




		glfwSwapBuffers(window);
		glfwPollEvents();

		double frameEnd = glfwGetTime();
		double frameDuration = frameEnd - currentFrame;
		if (frameDuration < FRAME_DURATION) {
			std::this_thread::sleep_for(std::chrono::milliseconds(
				static_cast<int>((FRAME_DURATION - frameDuration) * 1000)));
		}
	}

	Cleanup();

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}




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
