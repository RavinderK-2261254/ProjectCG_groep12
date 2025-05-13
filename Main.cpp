#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Bezier.h"
#include "Camera.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
std::vector<Bezier> BezierLooptieLoop(int bezCount, float radius, glm::vec3 centerPoint);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    Shader Kevin("line.vert", "line.frag");
    std::vector<Bezier> AllBeziers = BezierLooptieLoop(6, 4.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(5.0f, 0.0f, 5.0f));
    //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        // -----
        processInput(window);
        camera.Inputs(window,deltaTime);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Kevin.use();
        camera.Matrix(45.0f, 0.1f, 100.0f, Kevin, "cameraMatrix");
        for (auto& bezier : AllBeziers)
        { bezier.DrawBezier(Kevin);}
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
std::vector<Bezier> BezierLooptieLoop(int bezCount, float radius, glm::vec3 centerPoint) {
    std::vector<Bezier> beziers;
    std::vector<glm::vec3> Points;
    bool y_value = true;
    for (int i = 0; i < bezCount; i++) {
        float angle = (2.0f * glm::pi<float>() * i) / bezCount;
        float x = centerPoint.x + radius * cos(angle);
        float y = centerPoint.y + sin(angle * 3.0f) * 3.0f;
        float z = centerPoint.z + radius * sin(angle);
        if (y_value) {
            y = -y;
            y_value = false;
        }
        y_value = !y_value;
        Points.emplace_back(x, y, z);
    }
    for (int i = 0; i < bezCount; i++) {
        glm::vec3 firstpoint = Points[i];
        glm::vec3 lastPoint = Points[(i + 1) % bezCount];
        beziers.emplace_back(firstpoint, lastPoint);
    }
    return beziers;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}