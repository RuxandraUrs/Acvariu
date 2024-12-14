#include "Mesh.h"

Mesh::Mesh(std::string name, const vector<Vertex>& vertices, const vector<unsigned int>& indices, const vector<Texture>& textures)
{
   this->name = name;
   std::cout << "start mesh constructor " << std::endl;

   std::cout << "reserve vertices " << std::endl;
   numVertices = vertices.size();
   this->vertices.reset(new Vertex[numVertices]);

   std::cout << "start copy vertices " << std::endl;
   for (size_t i = 0; i < vertices.size(); ++i) {
      this->vertices.get()[i] = vertices[i];
   }

   std::cout << "reserve indices " << std::endl;
   numIndexes = indices.size();
   this->indices.reset(new unsigned int[numIndexes]);

   std::cout << "start copy indices " << std::endl;
   for (size_t i = 0; i < indices.size(); ++i) {
      this->indices.get()[i] = indices[i];
   }

   std::cout << "start copy textures " << std::endl;
   this->textures = textures;

   // now that we have all the required data, set the vertex buffers and its attribute pointers.
   setupMesh();

   std::cout << "end mesh constructor " << std::endl;
}

Mesh::Mesh(std::string name, unsigned int numVertices, std::shared_ptr <Vertex> vertices, unsigned int numIndexes, std::shared_ptr <unsigned int> indices, const vector<Texture>& textures)
{
   this->name = name;
   std::cout << "start mesh constructor. num vertice = " << numVertices << " num indexes " << numIndexes << std::endl;

   this->numVertices = numVertices;
   this->vertices = vertices;

   this->numIndexes = numIndexes;
   this->indices = indices;

   this->textures = textures;

   // now that we have all the required data, set the vertex buffers and its attribute pointers.
   setupMesh();

   std::cout << "end mesh constructor " << std::endl;
}

// render the mesh
void Mesh::Draw(Shader& shader)
{
   //std::cout << "start drawing " << std::endl;

   // bind appropriate textures
   unsigned int diffuseNr = 1;
   unsigned int specularNr = 1;
   unsigned int normalNr = 1;
   unsigned int heightNr = 1;
   for (unsigned int i = 0; i < textures.size(); i++)
   {
      glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
      // retrieve texture number (the N in diffuse_textureN)
      string number;
      string name = textures[i].type;
      if (name == "texture_diffuse")
         number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
         number = std::to_string(specularNr++); // transfer unsigned int to string
      else if (name == "texture_normal")
         number = std::to_string(normalNr++); // transfer unsigned int to string
      else if (name == "texture_height")
         number = std::to_string(heightNr++); // transfer unsigned int to string

     // now set the sampler to the correct texture unit
      std::string textureName = (name + number);
      int location = glGetUniformLocation(shader.ID, textureName.c_str());
      if (location != -1) {
         glUniform1i(location, i);
         // and finally bind the texture
         glBindTexture(GL_TEXTURE_2D, textures[i].id);
      }
   }

   // draw mesh
   glBindVertexArray(VAO);

   //std::cout << "draw triangles: " << numIndexes << std::endl;
   glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(numIndexes), GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);

   // always good practice to set everything back to defaults once configured.
   glActiveTexture(GL_TEXTURE0);
   //std::cout << "end drawing " << std::endl;
}

// initializes all the buffer objects/arrays
void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex positions, normals, texture coords, tangents, and bitangents
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), &vertices.get()[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndexes * sizeof(unsigned int), &indices.get()[0], GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    // Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}


