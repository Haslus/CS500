/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "pch.h"
#include "shapes.h"

Sphere parse_sphere(const std::string * lines);
Box parse_box(const std::string * lines);
Light parse_light(const std::string * lines);
Material parse_material(const std::string * lines);

class Scene
{
public:
	Scene() = default;
	Scene(const std::string & filepath, int width, int height, std::string output_name = "Out.png");
	void Setup();
	void Intersect(const Ray & ray);
	void GenerateRays();
	void GenerateImage();

	//Window Stuff
	void InitializeWindow();
	void UpdateWindow();

	GLFWwindow* window;
	//

	std::vector<Base*> objects;
	std::vector<Sphere> spheres;
	std::vector<Box> boxes;
	std::vector<Light> lights;
	vec3 global_ambient;
	std::vector<Ray> rays;

	int width;
	int height;

	Camera camera;

	std::string input_name;
	std::string output_name;
	std::vector<vec3> intersection_data;

	int samples;
	float epsilon;
	bool useHS;
	bool useSS;
};