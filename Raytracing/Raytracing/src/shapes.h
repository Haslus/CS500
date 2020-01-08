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