#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// 输出到片元着色器
out vec3 FragPos;
out vec3 Normal;
// out vec2 TexCoords; //暂时不用，但先留着

// MVP 矩阵
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 在世界空间中计算片元位置和法线
    FragPos = vec3(model * vec4(aPos, 1.0));
    // 使用法线矩阵 (model的逆转置矩阵) 来变换法线
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // 最终的裁剪空间位置
    gl_Position = projection * view * vec4(FragPos, 1.0);
}