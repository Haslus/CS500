/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/



#include "scene.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm\gtc\random.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

vec2 offset;
std::string m_win_name;
ImGuiWindowFlags m_flags;
bool updateShader = false;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void*)window;
	offset = { xoffset,yoffset };
}

/***********************************************

  @brief 	Initialize ImGUI

************************************************/
bool Scene::initImGUI()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
		return false;

	const char* imgui_version = "#version 130";

	if (!ImGui_ImplOpenGL3_Init(imgui_version))
		return false;

	m_win_name = "Editor";

	m_flags = 0;

	return true;
}
/**
* @brief 	Update ImGUI
*/
void Scene::updateImGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	renderImGUI();
	//ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
/**
* @brief 	Render ImGUI
*/
void Scene::renderImGUI()
{

	ImGui::Begin("Scene", nullptr, m_flags);
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	static char str[99];
	ImGui::InputText("Scene Name", str,30);
	if (ImGui::Button("Save Scene"))
	{
		CSG_Manager.SaveScene(str);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Scene"))
	{
		CSG_Manager.LoadScene(str);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Scene"))
	{
		CSG_Manager.CSGoperations.clear();
		CSG_Manager.CSGshapes.clear();
	}

	if (ImGui::Button("No Operations / Simple Light"))
	{
		renderShader = NoOperationsSimpleLightShader;

	}

	if (ImGui::Button("No Operations"))
	{
		renderShader = NoOperationsShader;
	}

	if (ImGui::Button("Operations / Simple Light"))
	{
		renderShader = OperationsSimpleLightShader;
	}


	if(ImGui::Button("Add Shape"))
	{
		CSGManager::CSGShape sphere1{ CSGManager::CSGShapeType::Sphere ,vec3(0,0,0),vec3(1.0,1.0,1.0),vec3(0,0,0) };
		CSG_Manager.CSGshapes.push_back(sphere1);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Operation"))
	{
		CSGManager::CSGOperation op1{ CSGManager::Difference,0,1 };
		CSG_Manager.CSGoperations.push_back(op1);
	}

	if (ImGui::TreeNode("Shapes"))
	{
		for (int i = 0; i < CSG_Manager.CSGshapes.size(); i++)
			if (ImGui::TreeNode((void*)(intptr_t)i, "Shape %d", i))
			{
				int shape = static_cast<int>(CSG_Manager.CSGshapes[i].type);

				const char * items1[11] = { "Sphere","Box","Torus","RoundBox","HexagonalPrism",
					"TriangularPrism", "Capsule","CappedCylinder", "RoundedCylinder", "CappedCone",
				"Octahedron"};
				ImGui::Combo("Shapes", &shape, items1, 11);
				CSG_Manager.CSGshapes[i].type = static_cast<CSGManager::CSGShapeType>(shape);


				ImGui::DragFloat3("Position", &CSG_Manager.CSGshapes[i].m_position.x, 0.1f);
				
				ImGui::DragFloat3("Scale", &CSG_Manager.CSGshapes[i].m_scale.x, 0.1f);

				ImGui::DragFloat3("Rotation", &CSG_Manager.CSGshapes[i].m_rotation.x, 0.1f);
					
				ImGui::TreePop();
			}
		ImGui::TreePop();
	}

	if (ImGui::Checkbox("Activate Operations", &CSG_Manager.noOperations));

	if (CSG_Manager.noOperations)
	{
		ImGui::DragFloat("Smooth Factor",&smoothFactor, 0.01);
		ImGui::DragFloat("Twist Factor", &twistFactor, 0.01);
		ImGui::DragFloat("Bend Factor", &bendFactor, 0.01);
		ImGui::DragFloat("Displacement Factor", &displacementFactor, 0.01);
	}

	if (ImGui::TreeNode("Operations"))
	{
		std::vector<std::string> strings;

		for (int i = 0; i < CSG_Manager.CSGshapes.size(); i++)
		{
			strings.push_back("Shape " + std::to_string(i));
		}

		for (int i = 0; i < CSG_Manager.CSGoperations.size(); i++)
		{
			strings.push_back("Operation " + std::to_string(i));
		}

		for (int i = 0; i < CSG_Manager.CSGoperations.size(); i++)
			if (ImGui::TreeNode((void*)(intptr_t)i, "Operation %d", i))
			{
				int & indexA = CSG_Manager.CSGoperations[i].indexA;
				int & indexB = CSG_Manager.CSGoperations[i].indexB;
				int op = static_cast<int>(CSG_Manager.CSGoperations[i].type);

				const char * items1[10] = { "Intersect","Union","Difference","SmoothIntersect","SmoothUnion","SmoothDifference",
				"Displacement", "Twist", "Bend","InfiniteRepetition" };
				ImGui::Combo("Operations", &op, items1, 10);
				CSG_Manager.CSGoperations[i].type = static_cast<CSGManager::CSGOperationType>(op);


				

				int current_item = indexA < 0 ? std::abs(indexA + 1) + CSG_Manager.CSGshapes.size() : indexA;

				if (ImGui::BeginCombo("Index A", strings[current_item].c_str())) 
				{
					for (int n = 0; n < strings.size(); n++)
					{
						bool is_selected = (strings[current_item] == strings[n]); 
						if (ImGui::Selectable(strings[n].c_str(), is_selected))
							current_item = n;
							if (is_selected)
								ImGui::SetItemDefaultFocus();  
					}
					ImGui::EndCombo();
				}

				indexA = current_item >= CSG_Manager.CSGshapes.size() ? -static_cast<int>(current_item - CSG_Manager.CSGshapes.size() + 1) : current_item;

				current_item = indexB < 0 ? std::abs(indexB + 1) + CSG_Manager.CSGshapes.size() : indexB;

				if (ImGui::BeginCombo("Index B", strings[current_item].c_str())) 
				{
					for (int n = 0; n < strings.size(); n++)
					{
						bool is_selected = (strings[current_item] == strings[n]); 
						if (ImGui::Selectable(strings[n].c_str(), is_selected))
							current_item = n;
						if (is_selected)
							ImGui::SetItemDefaultFocus();   
					}
					ImGui::EndCombo();
				}

				indexB = current_item >= CSG_Manager.CSGshapes.size() ? -static_cast<int>(current_item - CSG_Manager.CSGshapes.size() + 1) : current_item;



				
				ImGui::TreePop();
			}
		ImGui::TreePop();
	}

	ImGui::End();


}

/**
* @brief 	Exit ImGUI
*/
void Scene::exitImGUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

/***********************************************

	Create Window

************************************************/
void Scene::InitializeWindow()
{
	glfwSetErrorCallback(error_callback);

	/* Initialize the library */
	if (!glfwInit())
		throw std::invalid_argument("GLFW NOT INITIALIZED");


	//Generate Rays if needed

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "RAYTRACER", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		throw std::invalid_argument("WINDOW NOT CREATED");
	}
	
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	//renderShader = Shader{ "shaders/normal.vert","shaders/normal.frag" };
	NoOperationsSimpleLightShader = Shader{ "shaders/normal.vert","shaders/NoOperationsSimpleLight.frag" };
	NoOperationsShader = Shader{ "shaders/normal.vert","shaders/NoOperations.frag" };
	OperationsSimpleLightShader = Shader{ "shaders/normal.vert","shaders/OperationsSimpleLight.frag" };
	renderShader = NoOperationsSimpleLightShader;
	glfwSetScrollCallback(window, scroll_callback);
	
	initImGUI();

}

/***********************************************

	Update Window

************************************************/
void Scene::UpdateWindow()
{
	CSG_Manager.Setup();
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		GetInput();

		/* Render here */
		glClearColor(0.20f, 0.20f, 0.20f, 1.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		renderShader.Use();
		renderShader.SetVec2("Resolution", vec2(width, height));

		CSG_Manager.SetData(renderShader);

		renderShader.SetVec3("camEye", m_cam.camPos);
		renderShader.SetVec3("camFront", m_cam.camFront);
		renderShader.SetVec3("camUp", m_cam.camUp);
		renderShader.SetFloat("smoothFactor",smoothFactor);
		renderShader.SetFloat("twistFactor", twistFactor);
		renderShader.SetFloat("bendFactor", bendFactor);
		renderShader.SetFloat("displacementFactor",displacementFactor);
		RenderQuad();

		/* Swap front and back buffers */
		updateImGUI();
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	exitImGUI();
	
}
/***********************************************

	Create and Render a Quad for preview

************************************************/
void Scene::RenderQuad()
{
	if (quadVAO == -1)
	{
		unsigned int quadVBO;
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
/***********************************************

	Update the preview

************************************************/
void Scene::UpdateTexture()
{
	if (texture == -1)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		std::vector<glm::vec3> colorData;
		for (int i = 0; i < width * height; i++)
		{
			glm::vec3 color;
			color.r = 0.5;
			color.g = 0.0;
			color.b = 0.0;
			colorData.push_back(color);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, colorData.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, intersection_data.data());
	}
}

void Scene::GetInput()
{

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (glfwGetKey(window, GLFW_KEY_F5)) 
	{
		NoOperationsSimpleLightShader = Shader{ "shaders/normal.vert","shaders/NoOperationsSimpleLight.frag" };
		NoOperationsShader = Shader{ "shaders/normal.vert","shaders/NoOperations.frag" };
		OperationsSimpleLightShader = Shader{ "shaders/normal.vert","shaders/OperationsSimpleLight.frag" };
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
	{
		if (m_cam.orbital)
		{

			if (glfwGetKey(window, GLFW_KEY_Q)) {

				m_cam.Radius++;

			}
			if (glfwGetKey(window, GLFW_KEY_E)) {

				m_cam.Radius--;

			}

			m_cam.Radius -= offset.y;
			offset.y = 0;
			// View
			vec2 cursor_delta = { (float)xpos - m_mouse_position.x, (float)ypos - m_mouse_position.y };
			const float angleSpeed = 0.001f;
			m_cam.AngleX += cursor_delta.x * angleSpeed;
			const float max = glm::pi<float>() / 2.f;
			float delta = cursor_delta.y * angleSpeed;
			if (((m_cam.AngleY + delta) < max) && (m_cam.AngleY + delta) > (-max))
			{
				m_cam.AngleY += delta;
			}


		}

		else
		{
			float speed = 5.0f;


			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
				speed *= 3.0f;
			}
			if (glfwGetKey(window, GLFW_KEY_W)) {
				m_cam.camPos += glm::normalize(m_cam.camFront) * dt * speed;
			}
			if (glfwGetKey(window, GLFW_KEY_S)) {
				m_cam.camPos -= glm::normalize(m_cam.camFront) * dt * speed;
			}
			if (glfwGetKey(window, GLFW_KEY_A)) {
				m_cam.camPos -= glm::normalize(m_cam.camRight) * dt * speed;
			}
			if (glfwGetKey(window, GLFW_KEY_D)) {
				m_cam.camPos += glm::normalize(m_cam.camRight) * dt * speed;
			}
			if (glfwGetKey(window, GLFW_KEY_SPACE)) {
				m_cam.camPos += m_cam.camUp * dt * speed;
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
				m_cam.camPos -= m_cam.camUp * dt * speed;
			}

			vec2 cursor_delta = { (float)xpos - m_mouse_position.x, (float)ypos - m_mouse_position.y };
			const float angleSpeed = 0.1f;
			m_cam.axisAngle += vec3(cursor_delta.y * angleSpeed, cursor_delta.x * angleSpeed, 0);

			glm::quat q = glm::quat(glm::radians(m_cam.axisAngle));

			m_cam.quaternion = q;


			m_cam.RecalculateViewMatrix();
		}



	}

	m_mouse_position = { xpos,ypos };

	m_cam.RecalculateViewMatrix();

}