#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "RollerCoasterSpline.h" // Voeg deze toe

#include <iostream>
#include <vector>

// Functiedeclaraties
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void InitFloor();
std::vector<glm::vec3> GenerateLooptieLoopPoints(int count, float radius, glm::vec3 center);

// Floor
unsigned int floorVAO, floorVBO;

// Scherminstellingen
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // GLFW initialisatie
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rollercoaster Spline", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    InitFloor();

    Shader shader("line.vert", "line.frag");
    Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 15.0f));

    // Genereer looping spline
    std::vector<glm::vec3> loopPoints = GenerateLooptieLoopPoints(20, 6.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    RollerCoasterSpline coaster(loopPoints);

    while (!glfwWindowShouldClose(window))
    {
        // Time logica
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);
        camera.Inputs(window, deltaTime);

        // Render
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        camera.Matrix(45.0f, 0.1f, 100.0f, shader, "cameraMatrix");

        // Teken vloer
        glBindVertexArray(floorVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Teken spline
        coaster.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Keyboardinput
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Simpele vloer
void InitFloor()
{
    float floorVertices[] = {
        // positions          // colors
        -10.0f, -5.0f, -10.0f,  0.6f, 0.6f, 0.6f,
         10.0f, -5.0f, -10.0f,  0.6f, 0.6f, 0.6f,
         10.0f, -5.0f,  10.0f,  0.6f, 0.6f, 0.6f,
        -10.0f, -5.0f,  10.0f,  0.6f, 0.6f, 0.6f
    };
    unsigned int floorIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int EBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

std::vector<glm::vec3> GenerateLooptieLoopPoints(int count, float radius, glm::vec3 center)
{
    std::vector<glm::vec3> points;
    for (int i = 0; i < count; ++i) {
        float angle = glm::two_pi<float>() * i / count;
        float x = center.x + radius * cos(angle);
        float y = center.y + sin(angle * 3.0f) * 3.0f; // sinusgolf voor looping
        float z = center.z + radius * sin(angle);
        points.emplace_back(x, y, z);
    }
    return points;
}
