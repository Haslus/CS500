/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once
#include "shapes.h"

float intersection_ray_sphere(const Ray& ray, const Sphere& sphere);

float intersection_ray_box(const Ray& ray, const Box& box);

float intersection_ray_polygon(const Ray& ray, const SimplePolygon& poly);
IntersectionData intersection_ray_polygon_data(const Ray& ray, const SimplePolygon& poly);

float intersection_ray_triangle(const Ray& ray, const Triangle& tri);
IntersectionData intersection_ray_triangle_data(const Ray& ray, const Triangle& tri);

bool intersection_point_triangle(const vec3& point, const Triangle& tri);

float intersection_ray_ellipsoid(const Ray& ray, const Ellipsoid& ellip);
