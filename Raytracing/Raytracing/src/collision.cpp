/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#include "collision.h"

float intersection_ray_sphere(const Ray & ray, const Sphere & sphere)
{
	if (ray.dir == vec3(0, 0, 0))
		return -1;

	vec3 V = ray.start - sphere.center;
	float b = glm::dot(V, glm::normalize(ray.dir));
	float c = glm::dot(V, V) - sphere.radius * sphere.radius;

	if (c > 0.f && b > 0.f)
		return -1;

	float discr = b * b - c;

	if (discr < 0)
		return -1;

	float result = -b - glm::sqrt(discr);

	if (result < 0.f)
		result = 0.f;

	return result;
}

float intersection_ray_box(const Ray & ray, const Box & box)
{
	vec2 intersection = { 0.0f, FLT_MAX };

	for (int i = 0; i < 6; i++)
	{
		vec2 temp_intersection;
		float d2 = glm::dot(ray.dir, box.planes[i].normal);
		float d1 = glm::dot(ray.start - box.planes[i].point, box.planes[i].normal);
		
		if (d2 < 0.0)
		{
			float t = -d1 / d2;
			temp_intersection = {glm::max(0.0f,t),FLT_MAX};
		}
		else if (d2 > 0.0)
		{
			float t = -d1 / d2;
			temp_intersection = {0, glm::max(0.0f,t)};
		}
		else if (d1 > 0.0)
		{
			return -1;
		}
		else
		{
			temp_intersection = { 0.0f, FLT_MAX };
		}

		intersection = { glm::max(temp_intersection.x,intersection.x),
		glm::min(temp_intersection.y,intersection.y) };

	}

	if (intersection.x > intersection.y || intersection.x < 0 || intersection.y < 0)
		return -1;

	return intersection.x;
}
