#pragma once
#include "pch.h"

struct Camera
{
	Camera();
	void RecalculateViewMatrix();

	glm::mat4x4 ViewMatrix;
	vec3 camPos = vec3(0, 0, 5);
	vec3 camFront = vec3(0, 0, -1);
	vec3 camUp = vec3(0, 1, 0);
	vec3 camRight;
	glm::quat quaternion = glm::quat();
	glm::vec3 axisAngle = vec3(0, 0.0f, 0);
	bool orbital = false;


	float AngleX = 0;
	float AngleY = 0;
	float Radius = 30;
};