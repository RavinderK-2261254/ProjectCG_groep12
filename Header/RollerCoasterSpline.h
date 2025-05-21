#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"

//tekent de curve voor de rails
class RollerCoasterSpline {
public:
    RollerCoasterSpline(const std::vector<glm::vec3>& controlPoints);
    void Draw(Shader& shader);
    glm::vec3 GetPointOnTrack(float t) const;

private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> splinePoints;

    GLuint VAO, VBO;

    //berekent punten voor de curve zodat wannneer die punten worden verbonden, gaat het een vloeiende lijn worden
    glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
        const glm::vec3& p2, const glm::vec3& p3, float t) const;
    void GenerateSpline(int samplesPerSegment = 20);
    void BufferSetup();
};
