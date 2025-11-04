#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

// 构造函数
Mesh::Mesh(const std::string& path) {
    // 仅在加载成功时才 setup
    if (loadObj(path)) { 
        setupMesh();
    } else {
        std::cerr << "ERROR::MESH::Mesh construction failed for: " << path << std::endl;
    }
}

// Draw 函数
void Mesh::Draw(Shader &shader) {
    // 如果 VAO 没被创建 (因为加载失败)，就不要绘制
    if (VAO == 0) return; 
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

// setupMesh 函数 (不变)
void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 顶点属性指针
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}


// -----------------------------------------------------------------
//  !! 替换成这个更健壮的 .obj 加载器 !!
// -----------------------------------------------------------------
bool Mesh::loadObj(const std::string& path) {
    vertices.clear();

    std::vector<glm::vec3> temp_Positions;
    std::vector<glm::vec3> temp_Normals;
    std::vector<glm::vec2> temp_TexCoords;

    // 这是 .obj 解析器最难的部分：
    // .obj 格式允许 'f 1/1/1 2/2/2 3/3/3'
    // 'f 1//1 2//2 3//3'
    // 'f 1/1 2/2 3/3'
    // 'f 1 2 3'
    // 我们必须能处理所有这些情况

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR::MESH::Could not open file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_Positions.push_back(pos);
        } else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_Normals.push_back(norm);
        } else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_TexCoords.push_back(uv);
        } else if (prefix == "f") {
            std::string face_data_str;
            unsigned int vIdx, vtIdx, vnIdx;
            
            // 循环读取 "f" 行的每一个顶点 (v/t/n 组合)
            while (ss >> face_data_str) { // face_data_str 就像 "1/1/1" 或 "1//1"
                
                // --- 开始解析 "v/vt/vn" 字符串 ---
                std::stringstream face_ss(face_data_str);
                std::string segment;
                std::vector<std::string> segments;
                while(std::getline(face_ss, segment, '/')) {
                    segments.push_back(segment);
                }
                // --- 解析完毕 ---

                Vertex vertex;
                
                // .obj 索引从 1 开始, C++ 数组从 0 开始
                vIdx = std::stoul(segments[0]);
                vertex.Position = temp_Positions[vIdx - 1];

                // 检查格式
                if (segments.size() == 2) { // "f v/vt"
                    vtIdx = std::stoul(segments[1]);
                    vertex.TexCoords = temp_TexCoords[vtIdx - 1];
                    // (没有法线)
                } 
                else if (segments.size() == 3) {
                    if (segments[1].empty()) { // "f v//vn" (茶壶格式!)
                        vnIdx = std::stoul(segments[2]);
                        if (vnIdx > 0) { // 检查索引是否有效
                            vertex.Normal = temp_Normals[vnIdx - 1];
                            this->hasNormals = true; // !! <-- 在这里设置 "true" !!
                        }
                    } else { // "f v/vt/vn"
                        vtIdx = std::stoul(segments[1]);
                        vnIdx = std::stoul(segments[2]);
                        if (vtIdx > 0) vertex.TexCoords = temp_TexCoords[vtIdx - 1];
                        if (vnIdx > 0) { // 检查索引是否有效
                            vertex.Normal = temp_Normals[vnIdx - 1];
                            this->hasNormals = true; // !! <-- 也在这里设置 "true" !!
                        }
                    }
                }
                else if (segments.size() == 1) { // "f v" (只有位置)
                    // (没有纹理，没有法线)
                }

                vertices.push_back(vertex);
            }
        }
    }
    file.close();

    // 检查是否真的加载了顶点
    if (vertices.empty()) {
        std::cerr << "ERROR::MESH::No vertices loaded from file (is format supported?): " << path << std::endl;
        return false;
    }

    std::cout << "Loaded mesh: " << path << " with " << vertices.size() << " vertices." << std::endl;
    return true;
}