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
#include "shader.h"
#include "CSG.h"
#include "camera.h"


class Scene
{
public:
	//Raycasting
	Scene() = default;
	void Setup();

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
	
	float smoothFactor = 0.1;
	float twistFactor = 0.9;
	float bendFactor = 0.17;
	float displacementFactor = 0.96;

	int width = 1280;
	int height = 720;
	int threads = 1;

	std::string input_name;
	std::string output_name;
	std::vector<vec3> intersection_data;

	float epsilon = 0.0001f;

	
};

