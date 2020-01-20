/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#include "shapes.h"

Ray::Ray(const vec3 & start, const vec3 & dir)
{
	this->start = start;
	this->dir = dir;
}

Sphere::Sphere(const vec3 & center, const float & radius, const vec3& color)
{
	this->center = center;
	this->radius = radius;
	this->color = color;
}

Box::Box(const vec3 & position, const vec3 & weigth, const vec3 & heigth, const vec3 & length)
{
	this->position = position;
	this->width = width;
	this->heigth = heigth;
	this->length = length;
	//Front
	vec3 c1 = position;
	vec3 n1 = glm::cross(length, heigth);
	//Back
}

Plane::Plane(const vec3 & point, const vec3 & normal)
{
}
