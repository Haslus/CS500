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
#include <glm\gtx\euler_angles.hpp>
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
Sphere::Sphere(const vec3 & center, const float & radius, const Material& mat)
{
	this->center = center;
	this->radius = radius;
	this->mat = mat;
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
	vec3 intersection = ray.start + ray.dir * t;

	vec3 normal = glm::normalize(intersection - this->center);

	return normal;

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

}
/***********************************************

	Custom Constructor

************************************************/
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
SimplePolygon::SimplePolygon(const std::vector<vec3>& vertices, const std::vector<vec3>& indices,const Material & mat)
{
	this->vertices = vertices;
	this->mat = mat;
	this->number_of_vertices = vertices.size();


	for (int i = 0; i < indices.size(); i++)
	{
		triangles.push_back(Triangle( this->vertices[indices[i][0]],this->vertices[indices[i][1]], this->vertices[indices[i][2]] ));

	}
}
SimplePolygon::SimplePolygon(const std::vector<vec3>& vertices, const std::vector<vec3>& indices, const std::vector<vec3>& normals, 
	const std::vector<vec3>& idx_normals, const Material & mat)
{
	this->vertices = vertices;
	this->mat = mat;
	this->number_of_vertices = vertices.size();


	for (int i = 0; i < indices.size(); i++)
	{
		Triangle tri = Triangle(this->vertices[indices[i][0]],this->vertices[indices[i][1]], this->vertices[indices[i][2]]);

		triangles.push_back(tri);
	}
}
/***********************************************

	Intersect

************************************************/
float SimplePolygon::intersection(const Ray & ray)
{
	return intersection_ray_polygon(ray, *this);
}
IntersectionData SimplePolygon::intersection_data(const Ray & ray)
{
	return intersection_ray_polygon_data(ray, *this);
}
/***********************************************

	Get normal at the intersection

************************************************/
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
/***********************************************

	Custom Constructor

************************************************/
Ellipsoid::Ellipsoid(const vec3 & center, const vec3 & u, const vec3 & v, const vec3 & w, const Material & mat)
{
	this->center = center;
	this->u_vector = u;
	this->v_vector = v;
	this->w_vector = w;
	this->mat = mat;
}
/***********************************************

	Intersect

************************************************/
float Ellipsoid::intersection(const Ray & ray)
{
	return intersection_ray_ellipsoid(ray, *this);
}
IntersectionData Ellipsoid::intersection_data(const Ray & ray)
{
	IntersectionData data;
	data.t = intersection_ray_ellipsoid(ray, *this);
	data.PI = ray.start + ray.dir * data.t;
	glm::mat3x3 M{ u_vector, v_vector, w_vector };
	glm::mat3x3 inverseM = glm::inverse(M);

	data.normal = glm::normalize(glm::transpose(inverseM) * inverseM * (data.PI - center));

	return data;
}
/***********************************************

	Get normal at the intersection

************************************************/
vec3 Ellipsoid::normal_at_intersection(const Ray & ray, float t)
{
	vec3 P = ray.start + ray.dir * t;
	glm::mat3x3 M{ u_vector, v_vector, w_vector };
	glm::mat3x3 inverseM = glm::inverse(M);

	vec3 normal = glm::normalize(glm::transpose(inverseM) * inverseM * (P - center));

	return normal;
}
/***********************************************

	Custom Constructor

************************************************/
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
/***********************************************

	Sample a random point in a sphere

************************************************/
vec3 sample_sphere(const float & r)
{
	vec3 randP;
	randP.x = ((double)rand() / (RAND_MAX) - 0.5f);
	randP.y = ((double)rand() / (RAND_MAX) - 0.5f);
	randP.z = ((double)rand() / (RAND_MAX) - 0.5f);

	randP = glm::normalize(randP);
	float u = (double)rand() / (RAND_MAX);
	float c = std::cbrt(u);

	return randP * c * r;
}

Mesh::Mesh(const vec3& pos, const vec3& euler, const float& scale, const std::vector<vec3>& vertices,
	const std::vector<vec3>& faces, const std::vector<vec3>& normals, const std::vector<vec3>& idx_normals, const Material & mat)
{
	position = pos;
	euler_angles = euler;
	uniform_scale = scale;
	this->mat = mat;

	std::vector<vec3> transformed_verts,transformed_normals;
	mat4 RX = glm::rotate(glm::radians(euler.x), vec3(1, 0, 0));
	mat4 RY = glm::rotate(glm::radians(euler.y), vec3(0, 1, 0));
	mat4 RZ = glm::rotate(glm::radians(euler.z), vec3(0, 0, 1));
	mat4 R = RX * RY * RZ;
	mat4 M2W = glm::translate(position) * R * glm::scale(vec3(uniform_scale));


	for (int i = 0; i < vertices.size(); i++)
	{
		transformed_verts.push_back(vec3(M2W * vec4(vertices[i],1)));
	}

	poly = SimplePolygon(transformed_verts,faces,mat );
}

float Mesh::intersection(const Ray & ray)
{
	return poly.intersection(ray);
}

IntersectionData Mesh::intersection_data(const Ray & ray)
{
	return poly.intersection_data(ray);
}

vec3 Mesh::normal_at_intersection(const Ray & ray, float t)
{
	return poly.normal_at_intersection(ray, t);
}
