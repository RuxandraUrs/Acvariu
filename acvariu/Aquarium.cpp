#include "Aquarium.h"





Aquarium::Aquarium()
	{
		InitializeBuffers();
	}

Aquarium::~Aquarium()
	{
		Cleanup();
	}

	void Aquarium::RenderBottom(Shader& shader, const Camera& camera)
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

	void Aquarium::RenderGlass(Shader& shader, const Camera& camera)
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



	void Aquarium::RenderWater(Shader& shader, const Camera& camera, float time, const std::string& texturePath)
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

	void Aquarium::RenderCuboid(Shader& shader, const Camera& camera)
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

	void Aquarium::RenderAlgae(Shader& shader, const Camera& camera, float time, glm::vec3 basePosition, int numAlgae, glm::vec3 color) {
		shader.use();
		shader.setMat4("projection", camera.GetProjectionMatrix());
		shader.setMat4("view", camera.GetViewMatrix());
		shader.setFloat("time", time);

		float swaySpeed = 2.0f; // Speed of sway animation
		float swayAmplitude = glm::radians(5.0f); // Amplitude of sway
		float spreadAngle = glm::radians(360.0f / numAlgae); // Angle between algae

		for (int i = 0; i < numAlgae; ++i) {
			float angle = i * spreadAngle;
			float sway = swayAmplitude * glm::sin(time * swaySpeed + i);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, basePosition);
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(30.0f) + sway, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.05f, 0.3f, 0.05f)); // Small algae size

			shader.setMat4("model", model);
			shader.SetVec3("objectColor", color); // Set the color dynamically
			shader.SetVec3("viewPos", camera.GetPosition());
			shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);

			glBindVertexArray(algaeVAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, (20 + 1) * 2);
			glBindVertexArray(0);
		}
	}

	unsigned int Aquarium::LoadTexture(const std::string& filePath)
	{
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
	}



	void Aquarium::InitializeAlgaeBuffer() {
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

	void Aquarium::InitializeBuffers()
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

	void Aquarium::InitializeWaterBuffer() {
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


	void Aquarium::InitializesandBuffers()
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

	void Aquarium::Cleanup()
	{
		glDeleteVertexArrays(1, &VAO1);
		glDeleteBuffers(1, &VBO1);
		glDeleteVertexArrays(1, &VAO2);
		glDeleteBuffers(1, &VBO2);
		glDeleteVertexArrays(1, &VAO3);
		glDeleteBuffers(1, &VBO3);
	}
