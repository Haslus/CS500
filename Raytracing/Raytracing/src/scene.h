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
#include "shader.h"

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
	vec3 Intersect(const Ray & ray, const int&d = 1);
	void GenerateRays();
	void GenerateRaysRange(int begin, int end);
	void GenerateImage();

	//Window Stuff
	void InitializeWindow();
	void UpdateWindow();

	void RenderQuad();
	void UpdateTexture();

	GLFWwindow* window;

	unsigned int texture = -1;
	unsigned int quadVAO = -1;

	Shader renderShader;
	//

	std::vector<Base*> objects;
	std::vector<Sphere> spheres;
	std::vector<Box> boxes;
	std::vector<Light> lights;
	vec3 global_ambient;
	std::vector<Ray> rays;

	int width;
	int height;
	int threads = 5;

	Camera camera;

	std::string input_name;
	std::string output_name;
	std::vector<vec3> intersection_data;

	int samples;
	float epsilon;
	bool useHS;
	bool useSS;

	int max_depth = 5;

	//Air
	float air_electric_permittivity;
	float air_magnetic_permeability;
	vec3 air_attenuation;

	
};