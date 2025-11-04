#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// 包含我们自己的类
#include "Shader.h"
#include "Mesh.h"

// 包含 GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 包含 imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool isOrbitMode = true; // true = 轨道模式, false = 自由模式
bool isOrbitModePressed = false; // 用于按键防抖

// 光源
glm::vec3 lightPos   = glm::vec3(0.0, -10.0, -10.0);    // 光源位置
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);     // 光源颜色

bool isHeadLightMode = true; // true = 头灯模式, false = 固定光线模式
bool isHeadLightModePressed = false; // 用于按键防抖

// 自由视角
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);    // 摄像机位置
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);   // 摄像机正前方
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);    // 摄像机上方向
float cameraSpeed = 2.5f;

// 轨道摄像机
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);   // 相机的目标点
float cameraRadius = 3.0f;                              // 轨道半径 (从 cameraPos 初始化)

// 通用变量
float yaw   = -90.0f; // 俯仰
float pitch = 0.0f;   // 偏航
float roll = 0.0f;   // 偏航

// 鼠标状态变量
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 帧时间
float deltaTime = 0.0f;	// 这一帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间

// 声明回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// 重新读取防抖
bool isReloadPressed = false;
int main()
{
    // --- 1. 初始化 GLFW 和 GLAD ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OBJ Viewer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback); // 注册鼠标移动回调
    glfwSetScrollCallback(window, scroll_callback);   // 注册鼠标滚轮回调
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 捕捉鼠标
    

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST); 

    // 加载着色器
    std::string vsPath = std::string(RES_PATH) + "/shaders/obj_viewer.vs";
    std::string fsPath = std::string(RES_PATH) + "/shaders/obj_viewer.fs";
    Shader ourShader(vsPath.c_str(), fsPath.c_str());

    // 加载模型
    std::string objPath = std::string(RES_PATH) + "/models/teapot.obj";
    Mesh ourMesh(objPath.c_str());

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘
    
    // 设置 ImGui 风格
    ImGui::StyleColorsDark();

    // 设置 Platform/Renderer 后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 准备绘制新一帧ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        // 计算帧时间差
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 检查输入
        processInput(window); 


        // 热读取Shader
        if (isReloadPressed)
        {
            ourShader.reload();
            isReloadPressed = false; // 重置按键，防止每帧都 reload
        }
        
        // 清理
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        // 激活着色器
        ourShader.use();
        // 检查法线
        ourShader.setBool("u_hasNormals", ourMesh.hasNormals);
        // 设置 MVP 矩阵和 Uniform
        
        glm::mat4 view;
        
        // 根据模式计算 View 矩阵
        
        // 轨道模式
        if(isOrbitMode){
            // 计算位置
            cameraPos.x = cameraTarget.x + cameraRadius * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraPos.y = cameraTarget.y + cameraRadius * sin(glm::radians(pitch));
            cameraPos.z = cameraTarget.z + cameraRadius * sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            
            // 计算 View 矩阵
            view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        }

        // 自由模式
        else{
            // 鼠标控制相机朝向
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }

        // 把摄像机位置传给着色器
        ourShader.setVec3("viewPos", cameraPos);
        ourShader.setMat4("view", view);
        // 把光源信息传给着色器
        if(isHeadLightMode)lightPos = cameraPos;
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("lightColor", lightColor);


        // 透视投影矩阵
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // 模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ourShader.setMat4("model", model);

        // 绘制
        ourMesh.Draw(ourShader);

        //ImGui相关内容更新
        {
            // 创建一个新窗口
            ImGui::Begin("Camera Info");

            // 显示当前模式
            ImGui::Text("Camera Mode: %s", isOrbitMode ? "Orbit (Press C)" : "Free (Press C)");

            // 显示坐标
            ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
            
            if (isOrbitMode) {
                // 轨道模式下，显示 Yaw, Pitch, Radius
                ImGui::Text("Yaw: %.2f, Pitch: %.2f", yaw, pitch);
                ImGui::Text("Radius: %.2f", cameraRadius);
            } else {
                // 自由模式下，显示朝向
                ImGui::Text("Camera Front: (%.2f, %.2f, %.2f)", cameraFront.x, cameraFront.y, cameraFront.z);
                ImGui::Text("Camera Speed: (%.2f)", cameraSpeed);
            }

            // 结束窗口
            ImGui::End();
        }
        {
            // 创建一个新窗口
            ImGui::Begin("Light Info");

            // 显示当前模式
            ImGui::Text("Light Mode: %s", isHeadLightMode ? "HeadLight (Press X)" : "FixedLight (Press X)");

            // 显示坐标
            if (isHeadLightMode) {
                // 头灯模式直接显示光源位置
                ImGui::Text("Light Position: (%.2f, %.2f, %.2f)", lightPos.x, lightPos.y, lightPos.z);
            } else {
                // 固定光照模式，调整位置
                ImGui::DragFloat3("Light Position", glm::value_ptr(lightPos), 0.1f);
            }

            // 编辑颜色
            ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
            
            // 结束窗口
            ImGui::End();
        }
        // ImGui 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


    // 清理
    glfwTerminate();
    return 0;
}

// 回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

// 鼠标相应
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
        return;
    }
    // 防止与ImGui内容冲突
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    if (firstMouse){
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // 限制俯仰，防止欧拉角锁死
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    // 自由模式根据鼠标修改相机朝向
    if (!isOrbitMode){
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){

    // 防止与ImGui内容冲突
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
    
    if (isOrbitMode){
        // 轨道模式: 改变围绕半径
        cameraRadius -= (float)yoffset;
        if (cameraRadius < 1.0f)
            cameraRadius = 1.0f;
        if (cameraRadius > 45.0f)
            cameraRadius = 45.0f;
    }
    else{
        // 自由模式: 改变速度
        cameraSpeed += (float)yoffset * 0.5f; // 每次滚动增加/减少 0.5
        if (cameraSpeed < 0.5f)
            cameraSpeed = 0.5f;
        if (cameraSpeed > 10.0f)
            cameraSpeed = 10.0f;
    }
}

void processInput(GLFWwindow *window)
{
    // 退出
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Alt 释放/捕捉鼠标
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // C键切换模式
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !isOrbitModePressed){

        isOrbitMode = !isOrbitMode; // 翻转模式
        isOrbitModePressed = true;
        
        if (isOrbitMode) {
            // 刚切换到轨道模式:
            // 重新计算半径
            cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
            cameraRadius = glm::length(cameraPos - cameraTarget);
            // 重新计算朝向
            glm::vec3 direction = normalize(cameraPos - cameraTarget); // 这是从 Target 指向 Pos 的方向
            yaw = glm::degrees(atan2(direction.z, direction.x));
            pitch = glm::degrees(asin(direction.y));
        } 
        else {
            // 刚切换到自由模式:
            // 从当前位置/角度计算 cameraFront
            cameraFront = glm::normalize(cameraTarget - cameraPos);
            
            // 根据"视线方向"，反向推算 yaw 和 pitch
            yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x));
            pitch = glm::degrees(asin(cameraFront.y));
        }
    }
    
    // X键切换模式
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && !isHeadLightModePressed){

        isHeadLightMode = !isHeadLightMode; // 翻转模式
        isHeadLightModePressed = true;

    }


    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE){
        isOrbitModePressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE){
        isHeadLightModePressed = false;
    }

    // 自由模式下的 WASD 移动
    if (!isOrbitMode)
    {
        float speed = cameraSpeed * deltaTime; // 应用帧时间差
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
    
    // R键重新加载
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !isReloadPressed)
    {
        isReloadPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    {
        isReloadPressed = false;
    }
}