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
	vec3 diffuse_color;
	float specular_reflection;
	float specular_exponent;

	Material() = default;
	Material(const vec3& diffuse, const float & spec_ref, const float & spec_exp);

};

struct Ray
{
	vec3 start;
	vec3 dir;

	Ray() = default;
	Ray(const vec3& start, const vec3& dir);


};

struct Base
{
	Material mat;
	virtual float intersection(const Ray & ray) { return -1.0f; };
	virtual vec3 normal_at_intersection(const Ray & ray, float t) { return vec3();}
};


struct Sphere : public Base
{
	vec3	center;
	float	radius;

	Sphere() = default;
	Sphere(const vec3& center, const float & radius, const vec3& diffuse = vec3(), 
		const float & spec_ref = 0, const float & spec_exp = 0);
	float intersection(const Ray & ray);
	vec3 normal_at_intersection(const Ray & ray, float t);
	vec3 get_random_point();
};

struct Plane 
{
	vec3 point;
	vec3 normal;

	Plane() = default;
	Plane(const vec3& point, const vec3& normal);
};

struct Box : public Base
{
	vec3 position;
	vec3 width;
	vec3 height;
	vec3 length;
	//Front,Back,Left,Right,Bottom,Top
	Plane planes[6];

	Box() = default;
	Box(const vec3 & position, const vec3 & width, const vec3 & heigth,
		const vec3 & length, const vec3& diffuse, const float & spec_ref,
		const float & spec_exp);
	float intersection(const Ray & ray);
	vec3 normal_at_intersection(const Ray & ray, float t);
};

struct Light
{
	vec3 position;
	vec3 color;
	float radius;
	Light() = default;
	Light(const vec3 & position, const vec3 & color, const float & radius);
	//Physical representation of bulb
	Sphere bulb;
};


struct Camera
{

	vec3 center;
	vec3 up;
	vec3 right;
	float eye;
};