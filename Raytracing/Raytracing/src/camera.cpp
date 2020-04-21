#include "camera.h"

/**
* @brief 	Default constructor
*/
Camera::Camera()
{
	vec2 cursor_delta = { 0,0 };
	const float angleSpeed = 0.1f;
	axisAngle += vec3(cursor_delta.y * angleSpeed, cursor_delta.x * angleSpeed, 0);

	glm::quat q = glm::quat(glm::radians(axisAngle));

	quaternion = q;


	RecalculateViewMatrix();
}
/**
* @brief 	Recalculate view matrix
*/
void Camera::RecalculateViewMatrix()
{

	if (orbital)
	{
		camPos = glm::vec3(glm::sin(AngleX) * glm::cos(AngleY) * Radius, glm::sin(AngleY) * Radius, glm::cos(AngleX) * glm::cos(AngleY) * Radius);
	
		glm::vec3 fwdVec = glm::normalize(camPos);
		ViewMatrix = glm::lookAt(camPos, vec3(0, 0, 0), camUp);
	}
	else
	{
		glm::quat front = quaternion * glm::quat(0, 0, 0, -1) * glm::conjugate(quaternion);
		camFront = { front.x,front.y,front.z };
		glm::quat right = quaternion * glm::quat(0, 1, 0, 0) * glm::conjugate(quaternion);
		camRight = { right.x,right.y,right.z };
	
		glm::quat inverse = glm::conjugate(quaternion);
		glm::mat4 transInverse = glm::translate(-camPos);
	
	
		ViewMatrix = glm::toMat4(inverse) * transInverse;
	
	}

}
