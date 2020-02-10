/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#include "collision.h"
/***********************************************

	Intersection between ray and sphere

************************************************/
float intersection_ray_sphere(const Ray & ray, const Sphere & sphere)
{
	if (ray.dir == vec3(0, 0, 0))
		return -1.0f;

	vec3 v = ray.start - sphere.center;


	//Calculate A,B and C factors
	float a = glm::length(ray.dir) * glm::length(ray.dir);
	float b = 2.0f * glm::dot(ray.dir,v);
	float c = glm::length(v) *  glm::length(v) - sphere.radius * sphere.radius;
	//Calculate discriminant
	float discr = b * b - 4 * a * c;

	//if the discriminant is negative, no intersection
	if (discr < 0.0f)
		return -1.0f;
	else
	{
		float t1 = (-b + glm::sqrt(discr)) / (2.0f * a);
		float t2 = (-b - glm::sqrt(discr)) / (2.0f * a);

		//Else, there are three different outcomes
		if (t1 < 0.0f)
			return -1.0f;
		else if (t2 < 0.0f)
			return t1;
		else
			return t2;

	}
}
/***********************************************

	Intersection between ray and box

************************************************/
float intersection_ray_box(const Ray & ray, const Box & box)
{
	float tmin = 0;
	float tmax = FLT_MAX - 1;

	//Iterate for the six planes
	for (int i = 0; i < 6; i++)
	{
		float d2 = glm::dot(ray.dir, box.planes[i].normal);
		float d1 = glm::dot(ray.start - box.planes[i].point, box.planes[i].normal);
		

		if (d2 < 0.0)
		{
			float t = -d1 / d2;
			tmin = glm::max(tmin, t);
		}
		else if (d2 > 0.0)
		{
			float t = -d1 / d2;
			tmax = glm::min(tmax, t);
		}
		//No intersection
		else if (d1 > 0.0)
		{
			tmin = tmax + 1;
		}

	}

	if (tmin > tmax)
		return -1;

	if (tmin == 0)
		return tmax;
	else
		return tmin;
}
