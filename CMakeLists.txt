# (vcpkg 推荐至少 3.15)
cmake_minimum_required(VERSION 3.15)

project(OBJ_Viewer LANGUAGES CXX C)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
    src/main.cpp
    src/Shader.cpp
    src/Mesh.cpp
)


target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Stb_INCLUDE_DIR}

)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    glad::glad   
    glfw 
    glm::glm
    imgui::imgui
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    "RES_PATH=$<$<CONFIG:Debug>:\"../../res\">$<$<NOT:$<CONFIG:Debug>>:\"res\">"
)