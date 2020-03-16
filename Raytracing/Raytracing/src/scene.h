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

enum class AntiAliasing
{
	NONE, SUPER, ADAPTIVE
};

class Scene
{
public:
	Scene() = default;
	Scene(const std::string & filepath, int width, int height, std::string output_name = "Out.png");
	void Setup();
	vec3 Intersect(const Ray & ray, const int&d, const bool& transmitting);
	void GenerateRays();
	void GenerateRaysRange(int begin, int end);
	void GenerateImage();

	vec3 AdpativeASubdivision(vec3 P,vec3 corner, vec3 delta_right, vec3 delta_down, int recursion);

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
	int threads;

	Camera camera;

	std::string input_name;
	std::string output_name;
	std::vector<vec3> intersection_data;

	float epsilon;
	int shadowsamples;
	bool useHS;
	bool useSS;

	//Reflection
	int max_depth;
	int reflection_samples;

	//Air
	Material air;
	float air_electric_permittivity;
	float air_magnetic_permeability;
	bool use_attenuation;
	vec3 air_attenuation;

	//Aliasing
	AntiAliasing AA_method = AntiAliasing::NONE;
	int AA_samples = 4;
	int adaptive_AA_recursion = 4;
	float adaptive_AA_tolerance = 0.05f;

	
};