#include"Camera.h"
#define GLM_ENABLE_EXPERIMENTAL


Camera::Camera(int width, int height, glm::vec3 position, RollerCoasterSpline *coaster)
{
	Camera::width = width;
	Camera::height = height;
	Position = position;
	spline = coaster;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform)
{
	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Makes camera look in the right direction from the right position
	view = glm::lookAt(Position, Position + Orientation, Up);
	// Adds perspective to the scene
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	// Exports the camera matrix to the Vertex Shader
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
}



void Camera::Inputs(GLFWwindow* window, float deltaTime)
{
	// Handles key inputs
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Position += speed * Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Position += speed * -glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Position += speed * -Orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		Position += speed * Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		Position += speed * -Up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = 0.4f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Position += speed * deltaTime * Orientation * 10.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Position -= speed * deltaTime * Orientation * 10.0f;
	}
	
	static bool fKeyPressedLastFrame = false;
	//f ingedrukt
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressedLastFrame) {
		followTrack = !followTrack;
		fKeyPressedLastFrame = true;
	}
	//f losgelaten
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
		fKeyPressedLastFrame = false;
	}
	//als followtrack dan followSpline oproepen
	if (followTrack) {
		FollowSpline(deltaTime);
	}
	// Handles mouse inputs
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		// Hides mouse cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Prevents camera from jumping on the first click
		if (firstClick)
		{
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		// Stores the coordinates of the cursor
		double mouseX;
		double mouseY;
		// Fetches the coordinates of the cursor
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		// Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

		// Decides whether or not the next vertical Orientation is legal or not
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			Orientation = newOrientation;
		}

		// Rotates the Orientation left and right
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

		// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		// Unhides cursor since camera is not looking around anymore
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// Makes sure the next time the camera looks around it doesn't jump
		firstClick = true;
	}
}

void Camera::FollowSpline(float deltaTime)
{
	if (!spline) return;

	float speedAlongTrack = 0.1f;
	trackT += speedAlongTrack * deltaTime;
	if (trackT > 1.0f) trackT -= 1.0f;

	glm::vec3 position = spline->GetPointOnTrack(trackT);
	glm::vec3 tangent = spline->GetTangent(trackT);

	// Hardcoded global up (stabiliseert camera bij loopings)
	glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// Compute "side" and corrected up
	glm::vec3 side = glm::normalize(glm::cross(tangent, globalUp));
	glm::vec3 up = glm::normalize(glm::cross(side, tangent)); // corrected up

	Position = position - side * (0.5f / 2.0f) + up * 1.0f;
	Orientation = tangent;
	Up = up;
}
