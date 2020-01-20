/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "pch.h"

struct Material
{
	vec3 color;
	float shininess;

};

struct Ray
{
	vec3 start;
	vec3 dir;

	Ray() = default;
	Ray(const vec3& start, const vec3& dir);

};

struct Sphere {
	vec3	center;
	float	radius;
	vec3	color;

	Sphere() = default;
	Sphere(const vec3& center, const float & radius, const vec3& color);
};

struct Plane {
	vec3 point;
	vec3 normal;

	Plane() = default;
	Plane(const vec3& point, const vec3& normal);
};

struct Box {
	vec3 position;
	vec3 width;
	vec3 heigth;
	vec3 length;
	//Front,Back,Left,Right,Bottom,Top
	Plane planes[6];
	Material mat;

	Box() = default;
	Box(const vec3& position, const vec3& weigth, const vec3& heigth, const vec3& length);

};