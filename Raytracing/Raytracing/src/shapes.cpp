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

Ray::Ray(const vec3 & start, const vec3 & dir)
{
	this->start = start;
	this->dir = dir;
}

Sphere::Sphere(const vec3 & center, const float & radius, const vec3& diffuse,
	const float & spec_ref, const float & spec_exp)
{
	this->center = center;
	this->radius = radius;
	this->mat = Material{diffuse,spec_ref,spec_exp};
}

float Sphere::intersection(const Ray & ray)
{
	return intersection_ray_sphere(ray,*this);
}

vec3 Sphere::normal_at_intersection(const Ray & ray, float t)
{
	vec3 intersection = ray.start + glm::normalize(ray.dir) * t;

	vec3 normal = glm::normalize(intersection - this->center);

	return normal;

}

vec3 Sphere::get_random_point()
{
	vec3 random_vec = glm::normalize(vec3{rand() % 50 - 50,rand() % 50 - 50 ,rand() % 50 - 50 });
	return center + random_vec * radius;
	
}

Box::Box(const vec3 & position, const vec3 & length, const vec3 & width,
	const vec3 & height, const vec3& diffuse,const float & spec_ref, 
	const float & spec_exp)
{
	this->position = position;
	this->length = length;
	this->width = width;
	this->height = height;
	this->mat = Material{ diffuse,spec_ref,spec_exp };

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

float Box::intersection(const Ray & ray)
{
	return intersection_ray_box(ray, *this);
}

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

Plane::Plane(const vec3 & point, const vec3 & normal)
{
	this->point = point;
	this->normal = normal;
}

Material::Material(const vec3 & diffuse, const float & spec_ref, const float & spec_exp)
{
	diffuse_color = diffuse;
	specular_reflection = spec_ref;
	specular_exponent = spec_exp;
}

Light::Light(const vec3 & position, const vec3 & color, const float & radius)
{
	this->position = position;
	this->color = color;
	this->radius = radius;

	this->bulb = Sphere{position,radius};
}
