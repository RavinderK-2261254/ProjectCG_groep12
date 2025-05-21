#include "Bezier.h"
Bezier::Bezier(glm::vec3 firstPoint, glm::vec3 lastPoint) {
	glm::vec3 middle = (firstPoint + lastPoint) * 0.5f;
	glm::vec3 centerToMiddle = glm::normalize(middle);
	glm::vec3 direc = glm::normalize(lastPoint - firstPoint);
	glm::vec3 upper(0.0f, 1.0f, 0.0f);
	std::cout << "Direct: " << direc.x <<"|"<< direc.y <<"|" << direc.z << std::endl;
	if (glm::abs(glm::dot(direc,upper)) > 0.99f) {
		upper = glm::vec3(0.0f, 0.0f, 1.0f);
	}
	glm::vec3 offset = glm::normalize(glm::cross(direc, upper)) * 2.0f;
	float waveHeight = sin(glm::length(lastPoint - firstPoint)) * 2.0f;
	offset += glm::vec3(0.0f, waveHeight, 0.0f);
	//glm::vec3 offset(0.0f, 1.5f, 0.0f);
	glm::vec3 point1 = firstPoint + (lastPoint - firstPoint) * 0.25f + offset;
	glm::vec3 point2 = firstPoint + (lastPoint - firstPoint) * 0.75f + offset;

	controlpoints.push_back(firstPoint);
	controlpoints.push_back(point1);
	controlpoints.push_back(point2);
	controlpoints.push_back(lastPoint);
	generatingCurve();
	BufferSetup();
}
void Bezier::DrawBezier(Shader& shader) {
	shader.use();
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, curvepoints.size());
	glBindVertexArray(0);
}

glm::vec3 Bezier::casteljau(float a) {
	std::vector<glm::vec3> temp = controlpoints;
	for (int k = 1; k < temp.size(); k++) {
		for (int i = 0; i < temp.size() - k; i++) {
			temp[i] = (1.0f - a) * temp[i] + a * temp[i + 1];
		}
	}
	return temp[0];
}
void Bezier::BufferSetup() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, curvepoints.size() * sizeof(glm::vec3), &curvepoints[0],GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}
void Bezier::generatingCurve() {
	const int samples = 100;
	for (int i = 0; i <= samples; i++) {
		float t = (float)i / samples;
		curvepoints.push_back(casteljau(t));
	}
}
glm::vec3 Bezier::getEndPoint() const { return controlpoints.back(); }

