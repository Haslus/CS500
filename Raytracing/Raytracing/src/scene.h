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

class Scene
{
public:
	Scene(const std::string & filepath, int width, int height);
	void Intersect(const Ray & ray);
	void GenerateImage();
private:
	std::vector<Sphere> spheres;
	std::vector<Ray> rays;
	int width;
	int height;

	std::vector<vec3> intersection_data;
};