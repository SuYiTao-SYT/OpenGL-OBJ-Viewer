#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // 我们的着色器程序 ID
    unsigned int ID;

    // 着色器路径
    std::string vertexPath;
    std::string fragmentPath;

    // 构造函数: 读取并构建着色器
    Shader(const char* vertexPath, const char* fragmentPath);

    // 激活着色器
    void use();

    // uniform 工具函数 (setter)
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

    // 重新加载Shader
    void reload();

private:
    // 检查编译/链接错误的辅助函数
    void checkCompileErrors(unsigned int shader, std::string type);
};
#endif