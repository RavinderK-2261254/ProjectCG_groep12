#pragma once
#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>

class Bezier
{
public:
	Bezier(glm::vec3 firstPoint, glm::vec3 lastPoint);
	void DrawBezier(Shader& shader);
	glm::vec3 getEndPoint() const;
private:
	std::vector<glm::vec3> controlpoints;
	std::vector<glm::vec3> curvepoints;
	glm::vec3 casteljau(float a);
	unsigned int VAO, VBO;
	void BufferSetup();
	void generatingCurve();


};

