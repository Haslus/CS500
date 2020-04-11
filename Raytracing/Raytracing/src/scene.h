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
#include "CSG.h"
#include "camera.h"

Sphere parse_sphere(const std::string * lines);
Box parse_box(const std::string * lines);
Light parse_light(const std::string * lines);
Material parse_material(const std::string * lines);

enum class Shadows
{
	NONE, HARD, SOFT
};

enum class AntiAliasing
{
	NONE, SUPER, ADAPTIVE
};

class Scene
{
public:
	//Raycasting
	Scene() = default;
	Scene(const std::string & filepath, int width, int height, std::string output_name = "Out.png");
	void Setup();
	vec3 Raycast(const Ray & ray, const int&d, const bool& transmitting);
	void GenerateRays();
	void GenerateRaysRange(int begin, int end);
	void GenerateImage();

	vec3 AdpativeASubdivision(vec3 P,vec3 corner, vec3 delta_right, vec3 delta_down, int recursion);

	//Constructive Solid Geometry
	void GenerateRaysCSG(int begin, int end);

	CSGManager CSG_Manager;

	Camera m_cam;
	vec2 m_mouse_position;

	float dt = 0.0f;
	float lastFrame = 0.0f;

	/////

	//IMGUI
	bool initImGUI();
	void updateImGUI();
	void renderImGUI();
	void exitImGUI();


	//Window Stuff
	void InitializeWindow();
	void UpdateWindow();
	void GetInput();

	void RenderQuad();
	void UpdateTexture();

	GLFWwindow* window;

	unsigned int texture = -1;
	unsigned int quadVAO = -1;

	Shader renderShader;
	Shader NoOperationsSimpleLightShader;
	Shader NoOperationsShader;
	Shader OperationsSimpleLightShader;
	Shader currenShader;

	bool preview = true;
	//
	float smoothFactor = 0.1;
	float twistFactor = 0.9;
	float bendFactor = 0.17;
	float displacementFactor = 0.96;
	std::vector<Base*> objects;
	std::vector<Sphere> spheres;
	std::vector<Box> boxes;
	std::vector<Light> lights;
	vec3 global_ambient;
	std::vector<Ray> rays;

	int width = 1280;
	int height = 720;
	int threads = 1;

	std::string input_name;
	std::string output_name;
	std::vector<vec3> intersection_data;

	float epsilon = 0.0001f;

	//Shadows
	Shadows S_method;
	int shadowsamples;

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

