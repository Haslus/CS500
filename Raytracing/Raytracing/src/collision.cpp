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

	if (discr < cEpsilon)
		return -1;

	float result = -b - glm::sqrt(discr);

	if (result < 0.f)
		result = 0.f;

	return result;
}
