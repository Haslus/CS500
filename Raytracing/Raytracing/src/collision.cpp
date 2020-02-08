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

	vec3 v = ray.start - sphere.center;



	float a = glm::length(ray.dir) * glm::length(ray.dir);
	float b = 2.0f * glm::dot(ray.dir,v);
	float c = glm::length(v) *  glm::length(v) - sphere.radius * sphere.radius;

	float discr = b * b - 4 * a * c;

	if (discr < 0.0f)
		return -1.0f;
	else
	{
		float t1 = (-b + glm::sqrt(discr)) / (2.0f * a);
		float t2 = (-b - glm::sqrt(discr)) / (2.0f * a);


		if (t1 < 0.0f)
			return -1;
		else if (t2 < 0.0f)
			return t1;
		else
			return t2;

	}
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
			temp_intersection = {-1, glm::max(0.0f,t)};
		}
		else if (d1 > 0.0)
		{
			return -1;
		}
		else
		{
			temp_intersection = { -1, FLT_MAX };
		}

		intersection = { glm::max(temp_intersection.x,intersection.x),
		glm::min(temp_intersection.y,intersection.y) };

	}

	if (intersection.x > intersection.y)
		return -1;

	if (intersection.x < 0)
		return intersection.y;

	return intersection.x;
}
