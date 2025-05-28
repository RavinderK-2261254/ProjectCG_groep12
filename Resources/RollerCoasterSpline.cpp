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
        glm::vec3 p0 = controlPoints[(i + numPoints - 1) % numPoints];
        glm::vec3 p1 = controlPoints[i % numPoints];
        glm::vec3 p2 = controlPoints[(i + 1) % numPoints];
        glm::vec3 p3 = controlPoints[(i + 2) % numPoints];

        for (int j = 0; j < samplesPerSegment; ++j) {
            float t = static_cast<float>(j) / samplesPerSegment;
            glm::vec3 point = CatmullRom(p0, p1, p2, p3, t);
            splinePoints.push_back(point);
        }
    }

    // Geen extra punt nodig – cycliciteit wordt door modulo verzorgd
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
    int numSegments = controlPoints.size();
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
    t = glm::clamp(t, 0.0f, 0.9999f);
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

    std::vector<Vertex> leftRailVertices, rightRailVertices;
    std::vector<unsigned int> leftRailIndices, rightRailIndices;

    GenerateTubeRail(0.05f, 12, 100, leftRailVertices, leftRailIndices, true);
    GenerateTubeRail(0.05f, 12, 100, rightRailVertices, rightRailIndices, false);

    // 3D VAO voor linker rail
    glGenVertexArrays(1, &leftRailVAO3D);
    glGenBuffers(1, &leftRailVBO3D);
    glGenBuffers(1, &leftRailEBO3D);

    glBindVertexArray(leftRailVAO3D);
    glBindBuffer(GL_ARRAY_BUFFER, leftRailVBO3D);
    glBufferData(GL_ARRAY_BUFFER, leftRailVertices.size() * sizeof(Vertex), leftRailVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leftRailEBO3D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, leftRailIndices.size() * sizeof(unsigned int), leftRailIndices.data(), GL_STATIC_DRAW);

    // attrib layout 0: position, 1: normal, 2: texCoords
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    leftRailIndexCount = static_cast<int>(leftRailIndices.size());

    // 3D VAO voor rechter rail
    glGenVertexArrays(1, &rightRailVAO3D);
    glGenBuffers(1, &rightRailVBO3D);
    glGenBuffers(1, &rightRailEBO3D);

    glBindVertexArray(rightRailVAO3D);
    glBindBuffer(GL_ARRAY_BUFFER, rightRailVBO3D);
    glBufferData(GL_ARRAY_BUFFER, rightRailVertices.size() * sizeof(Vertex), rightRailVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightRailEBO3D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rightRailIndices.size() * sizeof(unsigned int), rightRailIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    rightRailIndexCount = static_cast<int>(rightRailIndices.size());

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
    float spacing = 0.4f; // afstand vanaf center
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

    DrawTubeRails(shader);
}

glm::vec3 RollerCoasterSpline::GetPointOnTrack(float t) const {
    t = glm::clamp(t, 0.0f, 0.9999f);
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

void RollerCoasterSpline::GenerateTubeRail(float radius, int circleSegments, int trackSamples,
    std::vector<Vertex>& outVertices,
    std::vector<unsigned int>& outIndices, bool isLeftRail)
{
    float railOffset = 0.5f;

    // Bereken parallel transport frames (up-vectors)
    std::vector<glm::vec3> upVectors = ComputeParallelTransportFrames(trackSamples);

    for (int i = 0; i <= trackSamples; ++i) {
        float t = static_cast<float>(i) / trackSamples;

        glm::vec3 center = GetPointOnTrack(t);
        glm::vec3 tangent = glm::normalize(GetTangent(t));

        glm::vec3 up = upVectors[i];
        glm::vec3 side = glm::normalize(glm::cross(tangent, up));
        glm::vec3 adjustedUp = glm::normalize(glm::cross(side, tangent));

        // Verplaats center naar linker of rechter rail
        glm::vec3 railCenter = center + (isLeftRail ? -side : side) * railOffset;

        // Voor ieder punt van de cirkel
        for (int j = 0; j < circleSegments; ++j) {
            float angle = 2.0f * glm::pi<float>() * j / circleSegments;
            glm::vec3 normal = cos(angle) * side + sin(angle) * adjustedUp;
            glm::vec3 position = railCenter + normal * radius;

            Vertex vertex;
            vertex.Position = position;
            vertex.Normal = normal;
            vertex.TexCoords = glm::vec2(float(j) / circleSegments, float(i) / trackSamples);
            outVertices.push_back(vertex);
        }
    }

    // Maak index buffer (triangels van de buis)
    for (int i = 0; i < trackSamples; ++i) {
        for (int j = 0; j < circleSegments; ++j) {
            int curr = i * circleSegments + j;
            int next = curr + circleSegments;
            int nextJ = (j + 1) % circleSegments;

            int currNext = i * circleSegments + nextJ;
            int nextNext = (i + 1) * circleSegments + nextJ;

            outIndices.push_back(curr);
            outIndices.push_back(next);
            outIndices.push_back(currNext);

            outIndices.push_back(next);
            outIndices.push_back(nextNext);
            outIndices.push_back(currNext);
        }
    }

}


void RollerCoasterSpline::DrawTubeRails(Shader& shader)
{
    shader.use();

    glBindVertexArray(leftRailVAO3D);
    glDrawElements(GL_TRIANGLES, leftRailIndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(rightRailVAO3D);
    glDrawElements(GL_TRIANGLES, rightRailIndexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

std::vector<glm::vec3> RollerCoasterSpline::ComputeParallelTransportFrames(int trackSamples)
{
    std::vector<glm::vec3> upVectors(trackSamples + 1);

    // Start vector up (kan anders als je wilt)
    glm::vec3 firstTangent = glm::normalize(GetTangent(0.0f));
    glm::vec3 arbitrary = glm::abs(glm::dot(firstTangent, glm::vec3(0, 1, 0))) > 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    glm::vec3 prevUp = glm::normalize(glm::cross(firstTangent, glm::cross(arbitrary, firstTangent)));
    upVectors[0] = prevUp;

    for (int i = 1; i <= trackSamples; ++i)
    {
        float tPrev = float(i - 1) / trackSamples;
        float tCurr = float(i) / trackSamples;

        glm::vec3 tangentPrev = glm::normalize(GetTangent(tPrev));
        glm::vec3 tangentCurr = glm::normalize(GetTangent(tCurr));

        glm::vec3 axis = glm::cross(tangentPrev, tangentCurr);

        if (glm::length(axis) < 1e-5f) {
            // nauwelijks draaiing
            upVectors[i] = prevUp;
        }
        else {
            axis = glm::normalize(axis);
            float angle = acos(glm::clamp(glm::dot(tangentPrev, tangentCurr), -1.0f, 1.0f));

            glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), angle, axis);
            glm::vec4 rotatedUp = rotMat * glm::vec4(prevUp, 0.0f);
            upVectors[i] = glm::vec3(rotatedUp);
            prevUp = upVectors[i];
        }
    }

    // **Hier corrigeren we de twist zodat upVectors[trackSamples] == upVectors[0]**

    glm::vec3 firstUp = upVectors[0];
    glm::vec3 lastUp = upVectors[trackSamples];

    float dot = glm::clamp(glm::dot(firstUp, lastUp), -1.0f, 1.0f);
    float angle = acos(dot);

    if (angle > 1e-5f)
    {
        // Rotatie-as tussen lastUp en firstUp
        glm::vec3 axis = glm::normalize(glm::cross(lastUp, firstUp));

        // Verdeel twist over alle frames
        for (int i = 0; i <= trackSamples; ++i)
        {
            float alpha = float(i) / trackSamples; // van 0 tot 1
            float currAngle = alpha * angle;

            glm::mat4 fixRot = glm::rotate(glm::mat4(1.0f), currAngle, axis);
            upVectors[i] = glm::vec3(fixRot * glm::vec4(upVectors[i], 0.0f));
        }
    }

    return upVectors;
}

