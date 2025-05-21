#include "RollerCoasterSpline.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>


RollerCoasterSpline::RollerCoasterSpline(const std::vector<glm::vec3>& points)
    : controlPoints(points) {
    GenerateSpline(50);
    BufferSetup();
}

glm::vec3 RollerCoasterSpline::CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
    const glm::vec3& p2, const glm::vec3& p3, float t) const {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

//genereert splinepunten door CatmulRom()
void RollerCoasterSpline::GenerateSpline(int samplesPerSegment) {
    splinePoints.clear();

    size_t numPoints = controlPoints.size();
    if (numPoints < 4) return;

    for (size_t i = 0; i < numPoints; ++i) {
        // Gebruik modulo om indices netjes om te wrappen, zodat de lus sluit
        glm::vec3 p0 = controlPoints[(i + numPoints - 1) % numPoints];
        glm::vec3 p1 = controlPoints[i % numPoints];
        glm::vec3 p2 = controlPoints[(i + 1) % numPoints];
        glm::vec3 p3 = controlPoints[(i + 2) % numPoints];

        for (int j = 0; j < samplesPerSegment; ++j) {
            float t = (float)j / samplesPerSegment;
            glm::vec3 point = CatmullRom(p0, p1, p2, p3, t);
            splinePoints.push_back(point);
        }
    }

    // Sluit visueel de lus door het eerste punt nog eens toe te voegen
    splinePoints.push_back(splinePoints.front());
}





void RollerCoasterSpline::BufferSetup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(glm::vec3), splinePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void RollerCoasterSpline::Draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, splinePoints.size());
    glBindVertexArray(0);
}

glm::vec3 RollerCoasterSpline::GetPointOnTrack(float t) const {
    t = glm::clamp(t, 0.0f, 1.0f);
    int totalSegments = (controlPoints.size() - 3);
    float totalT = t * totalSegments;
    int segIndex = (int)totalT;
    float localT = totalT - segIndex;

    if (segIndex >= totalSegments)
        segIndex = totalSegments - 1;

    return CatmullRom(
        controlPoints[segIndex],
        controlPoints[segIndex + 1],
        controlPoints[segIndex + 2],
        controlPoints[segIndex + 3],
        localT
    );
}
