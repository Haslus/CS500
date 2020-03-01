/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#include "shapes.h"
#include "collision.h"
#include <glm\gtc\random.hpp>
/***********************************************

	Custom Constructor

************************************************/
Ray::Ray(const vec3 & start, const vec3 & dir)
{
	this->start = start;
	this->dir = dir;
}
/***********************************************

	Custom Constructor

************************************************/
Sphere::Sphere(const vec3 & center, const float & radius, const vec3& diffuse,
	const float & spec_ref, const float & spec_exp)
{
	this->center = center;
	this->radius = radius;
	this->mat = Material{diffuse,spec_ref,spec_exp};
}
/***********************************************

	Intersect

************************************************/
float Sphere::intersection(const Ray & ray)
{
	return intersection_ray_sphere(ray,*this);
}
/***********************************************

	Get normal at the intersection

************************************************/
vec3 Sphere::normal_at_intersection(const Ray & ray, float t)
{
	vec3 intersection = ray.start + glm::normalize(ray.dir) * t;

	vec3 normal = glm::normalize(intersection - this->center);

	return normal;

}
/***********************************************

	Get a random point from the sphere

************************************************/
vec3 Sphere::get_random_point()
{
	vec3 random_vec = glm::ballRand(radius);

	return center + random_vec;
	
}
/***********************************************

	Custom Constructor

************************************************/
Box::Box(const vec3 & position, const vec3 & length, const vec3 & width,
	const vec3 & height, const Material& mat)
{
	this->position = position;
	this->length = length;
	this->width = width;
	this->height = height;
	this->mat = mat;

	//Front
	vec3 c1 = position;
	vec3 n1 = glm::normalize(glm::cross(length, height));
	planes[0] = Plane{ c1,n1 };
	//Back
	vec3 c2 = position + width;
	vec3 n2 = -n1;
	planes[1] = Plane{ c2,n2 };
	//Left
	vec3 c3 = position;
	vec3 n3 = glm::normalize(glm::cross(height, width));
	planes[2] = Plane{ c3,n3 };
	//Right
	vec3 c4 = position + length;
	vec3 n4 = -n3;
	planes[3] = Plane{ c4,n4 };
	//Bottom
	vec3 c5 = position;
	vec3 n5 = glm::normalize(glm::cross(width, length));
	planes[4] = Plane{ c5,n5 };
	//Top
	vec3 c6 = position + height;
	vec3 n6 = -n5;
	planes[5] = Plane{ c6,n6 };
}
/***********************************************

	Intersect

************************************************/
float Box::intersection(const Ray & ray)
{
	return intersection_ray_box(ray, *this);
}
/***********************************************

	Get normal at the intersection point

************************************************/
vec3 Box::normal_at_intersection(const Ray & ray, float t)
{
	vec3 intersection = ray.start + ray.dir * t;

	for (int i = 0; i < 6; i++)
	{

		if (std::abs(glm::dot(intersection - planes[i].point, planes[i].normal)) < cEpsilon)
		{
			return planes[i].normal;
		}
	}
	return vec3();
}
/***********************************************

	Custom Constructor

************************************************/
Plane::Plane(const vec3 & point, const vec3 & normal)
{
	this->point = point;
	this->normal = normal;
}
/***********************************************

	Custom Constructor

************************************************/
Material::Material(const vec3 & diffuse, const float & spec_ref, const float & spec_exp,
	const vec3& att, const float & elec,
	const float& magn, const float& rough) 
{
	diffuse_color = diffuse;
	specular_reflection = spec_ref;
	specular_exponent = spec_exp;
	attenuation = att;
	electric_perimittivity = elec;
	magnetic_permeability = magn;
	roughness = rough;

}
/***********************************************

	Custom Constructor

************************************************/
Light::Light(const vec3 & position, const vec3 & color, const float & radius)
{
	this->position = position;
	this->color = color;
	this->radius = radius;

	this->bulb = Sphere{position,radius};
}

SimplePolygon::SimplePolygon(const std::vector<vec3>& vertices, const Material & mat)
{
	this->vertices = vertices;
	this->mat = mat;
	this->number_of_vertices = this->vertices.size();


	for (int i = 0; i < number_of_vertices - 2; i++)
	{
		triangles.push_back(Triangle{ this->vertices[0],this->vertices[i + 1], this->vertices[i + 2] });
	}
}

float SimplePolygon::intersection(const Ray & ray)
{
	return intersection_ray_polygon(ray, *this);
}

vec3 SimplePolygon::normal_at_intersection(const Ray & ray, float t)
{
	vec3 P = ray.start + ray.dir * t;

	for (auto tri : triangles)
	{
		if (intersection_point_triangle(P, tri))
			return tri.plane.normal;
	}

	return vec3(0, 0, 0);
}

Ellipsoid::Ellipsoid(const vec3 & center, const vec3 & u, const vec3 & v, const vec3 & w, const Material & mat)
{
	this->center = center;
	this->u_vector = u;
	this->v_vector = v;
	this->w_vector = w;
	this->mat = mat;
}

float Ellipsoid::intersection(const Ray & ray)
{
	return intersection_ray_ellipsoid(ray, *this);
}

vec3 Ellipsoid::normal_at_intersection(const Ray & ray, float t)
{
	vec3 P = ray.start + ray.dir * t;
	glm::mat3x3 M{ u_vector, v_vector, w_vector };
	glm::mat3x3 inverseM = glm::inverse(M);

	vec3 normal = glm::normalize(glm::transpose(inverseM) * inverseM * (P - center));

	return normal;
}

Triangle::Triangle(const vec3 & vertex0, const vec3 & vertex1, const vec3 & vertex2)
{
	vertex_0 = vertex0;
	vertex_1 = vertex1;
	vertex_2 = vertex2;

	vec3 a = vertex_1 - vertex_0;
	vec3 b = vertex_2 - vertex_0;

	vec3 normal = glm::normalize(glm::cross(a, b));

	plane = Plane{ vertex_0, normal };
}

vec3 sample_sphere(const float & r)
{
	vec3 randP;
	randP.x = (((double)rand() / (RAND_MAX)) - 0.5f);
	randP.y = (((double)rand() / (RAND_MAX)) - 0.5f);
	randP.z = (((double)rand() / (RAND_MAX)) - 0.5f);

	randP = glm::normalize(randP);
	float u = ((double)rand() / (RAND_MAX));
	float c = std::cbrt(u);

	return randP * u;
}
