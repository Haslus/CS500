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

float intersection_ray_polygon(const Ray & ray, const SimplePolygon & poly)
{
	for (auto tri : poly.triangles)
	{
		float t = intersection_ray_triangle(ray, tri);

		if (t != -1.0f)
			return t;
	}
	return-1.0f;
}

float intersection_ray_triangle(const Ray & ray, const Triangle & tri)
{
	float dtop = glm::dot((ray.start - tri.plane.point), tri.plane.normal);
	float dbot = glm::dot(ray.dir, tri.plane.normal);
	float t = -dtop / dbot;

	if (t >= 0)
	{
		vec3 intersection_point = ray.start + t * ray.dir;
		if (intersection_point_triangle(intersection_point, tri))
			return t;
		else
			return -1.0f;
	}
	else
	{
		return -1.0f;
	}
}

bool intersection_point_triangle(const vec3 & point, const Triangle & tri)
{
	vec3 v0 = tri.vertex_1 - tri.vertex_0;
	vec3 v1 = tri.vertex_2 - tri.vertex_0;
	vec3 v2 = point - tri.vertex_0;

	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);

	float denominator = d00 * d11 - d01 * d01;

	if (denominator == 0)
		return false;

	float v = (d11 * d20 - d01 * d21) / denominator;
	float w = (d00 * d21 - d01 * d20) / denominator;
	float u = 1 - v - w;

	if (v < 0.f || v > 1.f ||
		w < 0.f || w > 1.f ||
		u < 0.f || u > 1.f)
		return false;

	return true;

}

float intersection_ray_ellipsoid(const Ray & ray, const Ellipsoid & ellip)
{
	glm::mat3x3 M{ ellip.u_vector,ellip.v_vector,ellip.w_vector };
	glm::mat3x3 inverseM = glm::inverse(M);

	float a = glm::length2(inverseM * ray.dir);
	float b = glm::dot(2.0f * inverseM * (ray.start - ellip.center), inverseM * ray.dir);
	float c = glm::length2(inverseM * (ray.start - ellip.center)) - 1.0f;

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
