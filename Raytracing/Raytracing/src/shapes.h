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

vec3 sample_sphere(const float & r = 0.1f);

struct Material
{
	vec3 diffuse_color;
	float specular_reflection;
	float specular_exponent;
	vec3 attenuation;
	float electric_perimittivity;
	float magnetic_permeability;
	float roughness;


	Material() = default;
	Material(const vec3& diffuse, const float & spec_ref, const float & spec_exp,
		const vec3& attenuation = vec3(),const float & electric_perimittivity = 0,
		const float& magnetic_permeability = 0, const float& roughness = 0);

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
	Sphere(const vec3& center, const float & radius, const Material& mat);
	float intersection(const Ray & ray);
	vec3 normal_at_intersection(const Ray & ray, float t);
};

struct Plane 
{
	vec3 point;
	vec3 normal;

	Plane() = default;
	Plane(const vec3& point, const vec3& normal);
};

struct Triangle
{
	vec3 vertex_0;
	vec3 vertex_1;
	vec3 vertex_2;
	Plane plane;

	Triangle() = default;
	Triangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2);
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
		const vec3 & length, const Material& mat);
	float intersection(const Ray & ray);
	vec3 normal_at_intersection(const Ray & ray, float t);
};

struct SimplePolygon : public Base
{
	int number_of_vertices;
	std::vector<vec3> vertices;
	std::vector<Triangle> triangles;

	SimplePolygon() = default;
	SimplePolygon(const std::vector <vec3>& vertices, const Material& mat);
	SimplePolygon(const std::vector <vec3>& vertices, const std::vector <vec3>& indices, const Material& mat);
	float intersection(const Ray & ray);
	vec3 normal_at_intersection(const Ray & ray, float t);
};

struct Mesh : public Base
{
	vec3 position;
	vec3 euler_angles;
	float uniform_scale;

	SimplePolygon poly;

	Mesh() = default;
	Mesh(const vec3& pos,const vec3& euler, const float& scale, std::vector<vec3> vertices,
		std::vector<vec3> faces, const Material & mat);
	float intersection(const Ray & ray);
	vec3 normal_at_intersection(const Ray & ray, float t);
};

struct Ellipsoid : public Base
{
	vec3 center;
	vec3 u_vector,v_vector,w_vector;

	Ellipsoid() = default;
	Ellipsoid(const vec3& center, const vec3& u, const vec3& v, const vec3& w,
		const Material& mat);
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