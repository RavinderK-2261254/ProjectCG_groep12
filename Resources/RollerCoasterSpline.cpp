#include "RollerCoasterSpline.h"
#define GLM_ENABLE_EXPERIMENTAL
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

    Generate3DRails();
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

glm::vec3 RollerCoasterSpline::CatmullRomTangent(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
    float t2 = t * t;

    return 0.5f * ((-p0 + p2) +
        2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t +
        3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t2);

}

glm::vec3 RollerCoasterSpline::GetPoint(float t) {
    int numSegments = controlPoints.size() - 3;
    float segmentFloat = t * numSegments;
    int segment = static_cast<int>(segmentFloat);
    float localT = segmentFloat - segment;

    glm::vec3 p0 = controlPoints[segment];
    glm::vec3 p1 = controlPoints[segment + 1];
    glm::vec3 p2 = controlPoints[segment + 2];
    glm::vec3 p3 = controlPoints[segment + 3];

    return CatmullRom(p0, p1, p2, p3, localT);
}

glm::vec3 RollerCoasterSpline::GetTangent(float t) {
    int numSegments = controlPoints.size();
    float segmentFloat = t * numSegments;
    int segment = static_cast<int>(segmentFloat) % numSegments;
    float localT = segmentFloat - segment;

    glm::vec3 p0 = controlPoints[(segment + controlPoints.size() - 1) % controlPoints.size()];
    glm::vec3 p1 = controlPoints[segment % controlPoints.size()];
    glm::vec3 p2 = controlPoints[(segment + 1) % controlPoints.size()];
    glm::vec3 p3 = controlPoints[(segment + 2) % controlPoints.size()];

    return glm::normalize(CatmullRomTangent(localT, p0, p1, p2, p3));
}

void RollerCoasterSpline::Generate3DRails()
{
    leftRailPoints.clear();
    rightRailPoints.clear();

    float spacing = 0.5f; // spatie tussen de rails
    const int steps = 100;
    for (float i = 0.0f; i < steps ; ++i) {
        float t = static_cast<float>(i) / steps;
        // punt van de track is het midden tussen beide kanten
        glm::vec3 center = GetPointOnTrack(t);
        //
        glm::vec3 tangent = glm::normalize(GetTangent(t));
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::vec3 side = glm::normalize(glm::cross(tangent, up));
        glm::vec3 left = center - side * spacing;
        glm::vec3 right = center + side * spacing;

        leftRailPoints.push_back(left);
        rightRailPoints.push_back(right);
    }

    // linker rail
    glGenVertexArrays(1, &leftRailVAO);
    glGenBuffers(1, &leftRailVBO);
    glBindVertexArray(leftRailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leftRailVBO);
    glBufferData(GL_ARRAY_BUFFER, leftRailPoints.size() * sizeof(glm::vec3), leftRailPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    //rechter rail
    glGenVertexArrays(1, &rightRailVAO);
    glGenBuffers(1, &rightRailVBO);
    glBindVertexArray(rightRailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rightRailVBO);
    glBufferData(GL_ARRAY_BUFFER, rightRailPoints.size() * sizeof(glm::vec3), rightRailPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void RollerCoasterSpline::Draw3DRails(Shader& shader)
{
    shader.use();
    glBindVertexArray(leftRailVAO);
    glDrawArrays(GL_LINE_LOOP, 0, leftRailPoints.size());

    glBindVertexArray(rightRailVAO);
    glDrawArrays(GL_LINE_LOOP, 0, rightRailPoints.size());

    glBindVertexArray(0);
}

void RollerCoasterSpline::GenerateRailMesh(float width, float height, std::vector<Vertex>& outVertices, std::vector<unsigned int>& outIndices)
{
    float spacing = 0.4f; // afstand tussen linker en rechter rail
    int segmentCount = 100;
    float dt = 1.0f / segmentCount;

    for (int i = 0; i < segmentCount; ++i)
    {
        float t1 = i * dt;
        float t2 = (i + 1) * dt;

        glm::vec3 p1 = GetPointOnTrack(t1);
        glm::vec3 p2 = GetPointOnTrack(t2);
        glm::vec3 tangent = glm::normalize(p2 - p1);
        glm::vec3 up(0, 1, 0);
        glm::vec3 side = glm::normalize(glm::cross(tangent, up));

        glm::vec3 l1 = p1 - side * spacing;
        glm::vec3 r1 = p1 + side * spacing;
        glm::vec3 l2 = p2 - side * spacing;
        glm::vec3 r2 = p2 + side * spacing;

        glm::vec3 normal = glm::normalize(glm::cross(tangent, side));

        // Voeg 4 vertices toe per segment (2 per rail)
        Vertex v0, v1, v2, v3;
        v0.Position = l1; v0.Normal = normal; v0.TexCoords = { 0.0f, 0.0f };
        v1.Position = r1; v1.Normal = normal; v1.TexCoords = { 1.0f, 0.0f };
        v2.Position = r2; v2.Normal = normal; v2.TexCoords = { 1.0f, 1.0f };
        v3.Position = l2; v3.Normal = normal; v3.TexCoords = { 0.0f, 1.0f };

        int startIndex = outVertices.size();
        outVertices.push_back(v0);
        outVertices.push_back(v1);
        outVertices.push_back(v2);
        outVertices.push_back(v3);

        // Twee driehoeken per segment
        outIndices.push_back(startIndex);
        outIndices.push_back(startIndex + 1);
        outIndices.push_back(startIndex + 2);

        outIndices.push_back(startIndex);
        outIndices.push_back(startIndex + 2);
        outIndices.push_back(startIndex + 3);
    }
}


void RollerCoasterSpline::Draw(Shader& shader) {
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, splinePoints.size());
    glBindVertexArray(0);

    Draw3DRails(shader);
}

glm::vec3 RollerCoasterSpline::GetPointOnTrack(float t) const {
    t = glm::clamp(t, 0.0f, 1.0f);
    int totalSegments = controlPoints.size();
    float totalT = t * totalSegments;
    int segIndex = static_cast<int>(totalT) % totalSegments;
    float localT = totalT - segIndex;

    glm::vec3 p0 = controlPoints[(segIndex + controlPoints.size() - 1) % controlPoints.size()];
    glm::vec3 p1 = controlPoints[segIndex % controlPoints.size()];
    glm::vec3 p2 = controlPoints[(segIndex + 1) % controlPoints.size()];
    glm::vec3 p3 = controlPoints[(segIndex + 2) % controlPoints.size()];

    return CatmullRom(p0, p1, p2, p3, localT);
}
