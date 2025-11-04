#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    unsigned int VAO = 0, VBO = 0; // !! 在这里初始化为 0 !!

    bool hasNormals = false; //是否读取到法线

    Mesh(const std::string& path);
    void Draw(Shader &shader);

private:
    bool loadObj(const std::string& path); 
    void setupMesh();
};
#endif