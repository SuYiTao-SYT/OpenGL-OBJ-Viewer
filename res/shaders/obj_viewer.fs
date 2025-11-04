#version 330 core
out vec4 FragColor;

// 从顶点着色器接收
in vec3 FragPos;
in vec3 Normal;

// 从 C++ 接收
uniform vec3 viewPos; // 摄像机位置
uniform vec3 lightPos; // 光源位置
uniform bool u_hasNormals; // 原模型是否包含法线信息
uniform vec3 lightColor; // 光源颜色



void main()
{
    vec3 norm; // 最终要使用的法线

    // ---------------------------------
    // !!          核心逻辑         !!
    // ---------------------------------
    if (u_hasNormals)
    {
        // 1. 模型有法线：使用 VBO 传来的法线 (平滑着色)
        norm = normalize(Normal);
    }
    else
    {
        // 2. 模型没有法线：自己计算 (平面着色)
        vec3 dFdx_pos = dFdx(FragPos);
        vec3 dFdy_pos = dFdy(FragPos);
        norm = normalize(cross(dFdx_pos, dFdy_pos));
    }
    // ---------------------------------


    //  Blinn-Phong 光照
    vec3 objectColor = vec3(0.7, 0.7, 0.7); 

    // 环境光
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 128.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    // 最终颜色
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}