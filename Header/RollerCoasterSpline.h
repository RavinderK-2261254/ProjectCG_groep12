#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL

//tekent de curve voor de rails
class RollerCoasterSpline {
public:
    RollerCoasterSpline(const std::vector<glm::vec3>& controlPoints);
    void Draw(Shader& shader);
    glm::vec3 GetPointOnTrack(float t) const;
    //berekent punten voor de curve zodat wannneer die punten worden verbonden, gaat het een vloeiende lijn worden
    glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
        const glm::vec3& p2, const glm::vec3& p3, float t) const;
    void GenerateSpline(int samplesPerSegment = 20);
    void BufferSetup();

    // de punten van de rails berekenen
    glm::vec3 CatmullRomTangent(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    glm::vec3 GetPoint(float t);
    glm::vec3 GetTangent(float t);

    // 3D rails maken
    void Generate3DRails();
    void Draw3DRails(Shader& shader);

    // rail mesh genereren
    void GenerateRailMesh(float width, float height, std::vector<Vertex>& outVertices, std::vector<unsigned int>& outIndices);
    void GenerateTubeRail(float radius, int circleSegments, int trackSamples, std::vector<Vertex>& outVertices, std::vector<unsigned int>& outIndices, bool isLeftRail);
    void DrawTubeRails(Shader& shader);

    std::vector<glm::vec3> ComputeParallelTransportFrames(int trackSamples);

private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> splinePoints;

    GLuint VAO, VBO;

    std::vector<glm::vec3> leftRailPoints;
    std::vector<glm::vec3> rightRailPoints;
    std::vector<glm::vec3> crossBeamPoints;
    GLuint leftRailVAO, leftRailVBO;
    GLuint rightRailVAO, rightRailVBO;
    GLuint crossBeamVAO, crossBeamVBO;
    unsigned int leftRailVAO3D, leftRailVBO3D, leftRailEBO3D;
    unsigned int rightRailVAO3D, rightRailVBO3D, rightRailEBO3D;
    int leftRailIndexCount = 0;
    int rightRailIndexCount = 0;

    
};

