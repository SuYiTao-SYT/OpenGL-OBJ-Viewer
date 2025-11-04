#include "Shader.h"

// 构造函数
Shader::Shader(const char* vPath, const char* fPath)
{
    // 记住路径
    this->vertexPath = vPath;
    this->fragmentPath = fPath;
    this->ID = 0; // 初始化 ID 为 0 (非常重要!)

    // 编译
    reload();
}

// 重新加载着色器
void Shader::reload()
{
    // 从文件路径读取 GLSL 源码
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    
    try 
    {
        vShaderFile.open(this->vertexPath);
        fShaderFile.open(this->fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();       
        
        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();     
    }
    catch(std::ifstream::failure &e)
    {
        // 读取失败，不崩溃，只打印错误
        std::cout << "ERROR::SHADER::RELOAD::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        return; // 退出 reload，保留旧的着色器
    }
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 编译着色器
    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX"); 

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // 链接着色器程序
    unsigned int newID = glCreateProgram(); // 创建一个"备用"程序
    glAttachShader(newID, vertex);
    glAttachShader(newID, fragment);
    glLinkProgram(newID);

    // 检查"链接"是否成功
    int success;
    char infoLog[1024];
    glGetProgramiv(newID, GL_LINK_STATUS, &success);
    if(!success)
    {
        // 链接失败报错
        glGetProgramInfoLog(newID, 1024, NULL, infoLog);
        std::cout << "--- SHADER RELOAD FAILED (Link Error) ---\n" << infoLog << "\n-----------------------------------------" << std::endl;
        
        // 清除错误链接程序
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(newID);
        return; // 保留原有旧程序
    }

    // 链接成功
    
    // 删除原有程序
    if (this->ID != 0)
    {
        glDeleteProgram(this->ID);
    }
    
    // 替换
    this->ID = newID;

    // 清理掉不再需要的着色器对象
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    std::cout << "Shader reloaded successfully!" << std::endl;
}

// 激活着色器
void Shader::use() { 
    glUseProgram(ID); 
}

// 封装Uniform Setter函数

void Shader::setBool(const std::string &name, bool value) const{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

// 错误检查
void Shader::checkCompileErrors(unsigned int shader, std::string type){
    int success;
    char infoLog[1024];
    if(type != "PROGRAM"){
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else{
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}