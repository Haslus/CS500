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
#include "shapes.h"

Sphere parse_sphere(const std::string * lines);
Box parse_box(const std::string * lines);

class Scene
{
public:
	Scene(const std::string & filepath, int width, int height, std::string output_name = "Out.png");
	void Intersect(const Ray & ray);
	void GenerateImage();
private:
	std::vector<Sphere> spheres;
	std::vector<Box> boxes;
	std::vector<Ray> rays;
	int width;
	int height;
	std::string name;
	std::vector<vec3> intersection_data;
};