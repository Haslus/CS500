/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date: 1/8/2020
----------------------------------------------------------------------------------------------------------*/



#include "scene.h"
#include "collision.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <glm\gtc\random.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}


/***********************************************

	Extract a vec3 from a string

************************************************/
vec3 extract_vec3(std::string line)
{
	std::string x = line.substr(0, line.find(','));
	line = line.substr(line.find(',') + 1);
	std::string y = line.substr(0, line.find(','));
	line = line.substr(line.find(',') + 1);
	std::string z = line.substr(0);

	vec3 result;
	result.x = std::stof(x);
	result.y = std::stof(y);
	result.z = std::stof(z);
	return result;
}
/***********************************************

	Extract a face from a string

************************************************/
vec3 extract_face(std::string line)
{
	std::string x = line.substr(0, line.find('/'));
	line = line.substr(line.find('/') + 1);
	std::string y = line.substr(0, line.find('/'));
	line = line.substr(line.find('/') + 1);
	std::string z = line.substr(0);

	vec3 result;
	result.x = std::stoi(x);
	result.y = std::stoi(y);
	result.z = std::stoi(z);
	return result;
}
/***********************************************

	Parser of the scene

************************************************/
Scene::Scene(const std::string & filepath, int width, int height, std::string output_name)
{
	//Read Scene File
	{
		std::ifstream file;
		file.open(filepath);
		std::string line;

		if (!file)
		{
			std::cout << "Invalid path." << std::endl;
			std::abort();
		}

		this->width = width;
		this->height = height;

		while (std::getline(file, line))
		{
			//Ignore if it starts with #
			if (line[0] == '#')
				continue;

			if (line.find("SPHERE") != std::string::npos)
			{
				std::string line2;
				std::getline(file, line2);
				std::string lines[2] = { line,line2 };
				spheres.push_back(parse_sphere(lines));
				objects.push_back(new Sphere{ spheres.back() });
			}

			if (line.find("BOX") != std::string::npos)
			{
				std::string line2;
				std::getline(file, line2);
				std::string line3;
				std::getline(file, line3);
				std::string lines[3] = { line,line2,line3 };
				boxes.push_back(parse_box(lines));
				objects.push_back(new Box{ boxes.back() });

			}

			if (line.find("POLYGON") != std::string::npos)
			{
				
				std::string text_number = line.substr(line.find(' ') + 1, line.find('(') - line.find(' ') - 1);
				int number_of_vertices = std::stoi(text_number);
				std::vector<vec3> text_vertices;

				while (number_of_vertices > 0)
				{
					std::string text_vertex = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
					line = line.substr(line.find(')') + 1);
					text_vertices.push_back(extract_vec3(text_vertex));

					number_of_vertices--;

				}

				std::getline(file, line);

				objects.push_back(new SimplePolygon{ text_vertices,parse_material(&line) });
			}

			if (line.find("ELLIPSOID") != std::string::npos)
			{
				std::string text_center = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
				line = line.substr(line.find(')') + 1);
				std::string text_u = line.substr(2, line.find(')') - 2);
				line = line.substr(line.find(')') + 1);
				std::string text_v = line.substr(2, line.find(')') - 2);
				line = line.substr(line.find(')') + 1);
				std::string text_w = line.substr(2, line.find(')') - 2);


				vec3 center = extract_vec3(text_center);
				vec3 u = extract_vec3(text_u);
				vec3 v = extract_vec3(text_v);
				vec3 w = extract_vec3(text_w);

				std::getline(file, line);
				objects.push_back(new Ellipsoid{ center,u,v,w,parse_material(&line) });
			}

			if (line.find("MESH") != std::string::npos)
			{
				std::string filename = line.substr(line.find_first_of(' ') + 1);

				std::ifstream mesh_file;
				mesh_file.open(filename);
				std::string mesh_line;

				if (!mesh_file)
				{
					std::cout << "Invalid path." << std::endl;
					std::abort();
				}
				std::vector<vec3> vertices, indices;
				while (std::getline(mesh_file, mesh_line))
				{
					//Ignore if it starts with #
					if (mesh_line[0] == '#')
						continue;

					//Extract vertex
					if (mesh_line[0] == 'v' && mesh_line[1] == ' ')
					{
						mesh_line = mesh_line.substr(2);
						std::string vx = mesh_line.substr(0, mesh_line.find_first_of(' '));
						mesh_line = mesh_line.substr(mesh_line.find_first_of(' ') + 1);
						std::string vy = mesh_line.substr(0, mesh_line.find_first_of(' '));
						mesh_line = mesh_line.substr(mesh_line.find_first_of(' ') + 1);
						std::string vz = mesh_line;

						vec3 vertex{std::stof(vx),std::stof(vy), std::stof(vz)};
						vertices.push_back(vertex);
					}

					//Extract face
					else if (mesh_line[0] == 'f')
					{
						mesh_line = mesh_line.substr(2);
						std::string text_face_1 = mesh_line.substr(0, mesh_line.find_first_of(' '));
						mesh_line = mesh_line.substr(mesh_line.find_first_of(' ') + 1);
						std::string text_face_2 = mesh_line.substr(0, mesh_line.find_first_of(' '));
						mesh_line = mesh_line.substr(mesh_line.find_first_of(' ') + 1);
						std::string text_face_3 = mesh_line.substr(0, mesh_line.find_first_of(' '));

						vec3 face_1 = extract_face(text_face_1);
						vec3 face_2 = extract_face(text_face_2);
						vec3 face_3 = extract_face(text_face_3);

						indices.push_back(face_1);
						indices.push_back(face_2);
						indices.push_back(face_3);
					}

				}

				std::getline(file, line);
				std::string text_pos = line.substr(1, line.find_first_of(')') - 1);
				line = line.substr(line.find_first_of(' ') + 1);
				std::string text_angle = line.substr(1, line.find_first_of(')') - 1);
				line = line.substr(line.find_first_of(' ') + 1);
				std::string text_scale = line;

				std::getline(file, line);
			;

				Mesh * mesh = new Mesh(extract_vec3(text_pos),extract_vec3(text_angle),std::stof(text_scale),
					vertices,indices, parse_material(&line));


				objects.push_back(mesh);
			}

			if (line.find("LIGHT") != std::string::npos)
			{
				lights.push_back(parse_light(&line));
			}

			if (line.find("AMBIENT") != std::string::npos)
			{
				std::string text_ambient = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
				global_ambient = extract_vec3(text_ambient);
			}

			if (line.find("AIR") != std::string::npos)
			{
				line = line.substr(line.find(' ') + 1);
				std::string text_elec = line.substr(0,line.find(' '));
				line = line.substr(line.find(' ') + 1);
				std::string text_mag = line.substr(0, line.find(' '));
				line = line.substr(line.find(' ') + 1);
				std::string text_att = line.substr(line.find('(') + 1, line.find(')') - 1);

				air_electric_permittivity = std::stof(text_elec);
				air_magnetic_permeability = std::stof(text_mag);
				air_attenuation = extract_vec3(text_att);
			}

			if (line.find("CAMERA") != std::string::npos)
			{
				std::string text_center = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
				line = line.substr(line.find(')') + 1);
				std::string text_right = line.substr(2, line.find(')') - 2);
				line = line.substr(line.find(')') + 1);
				std::string text_up = line.substr(2, line.find(')') - 2);
				line = line.substr(line.find(')') + 1);
				std::string text_eye = line.substr(0);

				camera.center = extract_vec3(text_center);
				camera.up = extract_vec3(text_up);
				camera.right = extract_vec3(text_right);
				camera.eye = std::stof(text_eye);




			}


		}

		file.close();
		this->output_name = output_name;
		this->input_name = filepath;
	}
	

	//Read Config File
	{
		std::ifstream file;
		file.open("./config.txt");
		std::string line;

		if (!file)
		{
			std::cout << "Invalid path." << std::endl;
			std::abort();
		}

		while (std::getline(file, line))
		{
			//Ignore if it starts with #
			if (line[0] == '#')
				continue;

			if (line.find("HARD_SHADOWS") != std::string::npos)
			{
				if (line.find("true") != std::string::npos)
				{
					useHS = true;
				}
				else
				{
					useHS = false;
				}
			}

			if (line.find("SOFT_SHADOWS") != std::string::npos)
			{
				if (line.find("true") != std::string::npos)
				{
					useSS = true;
				}
				else
				{
					useSS = false;
				}
			}

			if (line.find("EPSILON") != std::string::npos)
			{
				std::string epsilon = line.substr(line.find('=') + 1);
				this->epsilon = std::stof(epsilon);
			}

			if (line.find("SHADOW_SAMPLES") != std::string::npos)
			{
				std::string shadowsamples = line.substr(line.find('=') + 1);
				this->shadowsamples = std::stoi(shadowsamples);
			}

			if (line.find("MAX_DEPTH") != std::string::npos)
			{
				std::string maxdepth = line.substr(line.find('=') + 1);
				this->max_depth = std::stoi(maxdepth);
			}

			if (line.find("REFLECTION_SAMPLES") != std::string::npos)
			{
				std::string reflection_samples = line.substr(line.find('=') + 1);
				this->reflection_samples = std::stoi(reflection_samples);
			}

			if (line.find("THREADS") != std::string::npos)
			{
				std::string THREADS = line.substr(line.find('=') + 1);
				this->threads = std::stoi(THREADS);
			}

			if (line.find("ATTENUATION") != std::string::npos)
			{
				if (line.find("true") != std::string::npos)
				{
					use_attenuation = true;
				}
				else
				{
					use_attenuation = false;
				}
			}
		}

		file.close();
	}
	
	
}
/***********************************************

	Setup the scene, by generating rays

************************************************/
void Scene::Setup()
{
	intersection_data = std::vector<vec3>(width * height, vec3{ 1,0,0 });
}
/***********************************************

	Throw a Ray

************************************************/
vec3 Scene::Intersect(const Ray & ray, const int& d, const float & n_i )
{
	//MAX DEPTH REACHED
	if (d < 0)
		return vec3{ 0, 0, 0 };

	float d_max = FLT_MAX - 1.f;
	int index = -1;
	float t = -1;
	//Intersect objects
	for (int i = 0; i < objects.size(); i++)
	{
		float d = objects[i]->intersection(ray);

		if (d >= 0.f && d <= d_max)
		{
			d_max = d;
			index = i;
			t = d;
		}

	}
	//NO INTERSECTION
	if (index == -1 || t == -1)
		return vec3{ 0,0,0 };

	Material material = objects[index]->mat;
	vec3 normal = objects[index]->normal_at_intersection(ray, t);
	vec3 color = global_ambient * material.diffuse_color;
	
	vec3 P = (ray.start + ray.dir * t) + epsilon * normal;
	vec3 viewVec = glm::normalize(ray.start - P);


	vec3 ID{ 0, 0, 0};
	vec3 IS{ 0, 0, 0};

	float n_t = 1;
	//TODO
	if (n_i == 1)
		n_t = 0; //Index of refraction of object at P

	//Reflection coefficient
	//TODO
	float R;
	//Specular reflection coefficient of object
	float ks = material.specular_reflection;
	R *= ks;

	//TODO CHANGE KS FOR R
	//APPLY LOCAL ILLUMINATION
	for (Light & light : lights)
	{
		//Calculate Diffuse + Specular
		vec3 lightDir = glm::normalize(light.position - P);

		float shadow_factor = 1;
		if (useHS)
		{
			//Calculate Hard Shadow
			Ray temp_ray{ P, lightDir };
			float distance = glm::length(light.position - P);

			for (int i = 0; i < objects.size(); i++)
			{
				float d = objects[i]->intersection(temp_ray);

				if (d != -1.0f && d <= distance)
				{
					vec3 point = temp_ray.start + temp_ray.dir * d;

					if (glm::length(point - P) <= distance)
					{
						shadow_factor = 0;
						break;

					}
				}

			}

		}	

		else if (useSS)
		{
			//Calculate Soft Shadows
			shadow_factor = 0;
			for (int s = 0; s < shadowsamples; s++)
			{
				vec3 randDir = light.position + sample_sphere(light.radius) - P;
				Ray ray{ P, glm::normalize(randDir) };
				float distance = glm::length(randDir);

				for (int i = 0; i < objects.size(); i++)
				{
					float d = objects[i]->intersection(ray);

					if (d != -1.0f && d <= distance)
					{
						vec3 point = ray.start + ray.dir * d;

						if (glm::length(point - P) <= distance)
						{
							shadow_factor++;
							break;
						}
					}

				}
			}

			shadow_factor = 1.0f - static_cast<float>(shadow_factor / shadowsamples);

		}

			

		//Calculate Diffuse factor
		ID += glm::max(glm::dot(normal, lightDir), 0.0f) * light.color * shadow_factor;

		//Calculate Specular factor
		vec3 reflection = glm::normalize(glm::reflect(-lightDir, normal));
		IS += glm::pow(glm::max(glm::dot(reflection, viewVec), 0.0f), material.specular_exponent) * light.color * shadow_factor;


	}

	color += material.diffuse_color * ID + material.specular_reflection * IS;
	
	//Use attenuation
	vec3 att = { 1.0f,1.0f,1.0f };

	if (use_attenuation)
	{
		att.x = glm::pow(air_attenuation.x, glm::length(ray.start - P));
		att.y = glm::pow(air_attenuation.y, glm::length(ray.start - P));
		att.z = glm::pow(air_attenuation.z, glm::length(ray.start - P));
	}

	if (R != 0.0f)
	{
		vec3 reflected_dir = glm::normalize(glm::reflect(ray.dir, normal));

		if (material.roughness > 0.0f)
		{
			vec3 reflec_color = vec3(0, 0, 0);

			for (int i = 0; i < reflection_samples; i++)
			{
				Ray reflected_ray{ P, glm::normalize(reflected_dir + sample_sphere(material.roughness)) };
				reflec_color += Intersect(reflected_ray, d - 1);
			}

			reflec_color /= reflection_samples;
			color = (color + R * reflec_color);
		}
		else
		{

			Ray reflected_ray{ P, reflected_dir };
			color = (color + R * Intersect(reflected_ray, d - 1,n_i));
		}
	}
	//Transmission coefficient of object at P
	//TODO
	float T;
	T *= ks;

	if (T != 0.0f)
	{
		//TODO
		vec3 transmitted_dir;
		Ray transmitted_ray{ P, transmitted_dir };
		color = (color + T * Intersect(transmitted_ray, d - 1, n_t));
	}

	return color;

	//Absorb
	//color *= 1.0f - material.specular_reflection;

	
}
/***********************************************

	Generate Rays

************************************************/
void Scene::GenerateRays()
{
	
	vec3 forward = glm::normalize(glm::cross(camera.up, camera.right));
	vec3 start = camera.center - forward * camera.eye;

	float halfWidth = static_cast<float>(width / 2);
	float halfHeigth = static_cast<float>(height / 2);

	//Generate rays
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			vec3 P = camera.center + static_cast<float>((j - halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i - halfHeigth + 0.5f) / halfHeigth) * camera.up;
			Ray ray{ start,glm::normalize(P - start) };
			rays.push_back(ray);
			intersection_data[i * width + j] = Intersect(ray,max_depth);

		}
}
/***********************************************

	Generate Rays from BEGIN height to END heigth

************************************************/
void Scene::GenerateRaysRange(int begin, int end)
{
	vec3 forward = glm::normalize(glm::cross(camera.up, camera.right));
	vec3 start = camera.center - forward * camera.eye;

	float halfWidth = static_cast<float>(width / 2);
	float halfHeigth = static_cast<float>(height / 2);

	//Generate rays
	for (int i = begin; i < end; i++)
		for (int j = 0; j < width; j++)
		{
			vec3 P = camera.center + static_cast<float>((j - halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i - halfHeigth + 0.5f) / halfHeigth) * camera.up;
			Ray ray{ start,glm::normalize(P - start) };
			intersection_data[i * width + j] = glm::clamp(Intersect(ray, max_depth), vec3(0), vec3(1));

		}
}

/***********************************************

	Store Image

************************************************/
void Scene::GenerateImage()
{
	std::vector<unsigned char> converted_data;
	for (auto data : intersection_data)
	{
		if (data.x > 1 || data.y > 1 || data.z > 1)
			continue;
		converted_data.push_back(static_cast<unsigned char>(data.x * 255.99f));
		converted_data.push_back(static_cast<unsigned char>(data.y * 255.99f));
		converted_data.push_back(static_cast<unsigned char>(data.z * 255.99f));
	}
	stbi_write_png(output_name.c_str(), width, height, 3, converted_data.data(),0);
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

	renderShader = Shader{ "shaders/normal.vert","shaders/normal.frag" };

	//Generate Rays if needed
	int division = width / threads;

	for (int i = 0; i < threads; i++)
	{
		if (i == threads - 1)
		{
			std::thread ray1(&Scene::GenerateRaysRange, this, division * i, width);
			ray1.detach();
		}
		else
		{
			std::thread ray1(&Scene::GenerateRaysRange,this, division * i, division * (i + 1));
			ray1.detach();
		}
	}



	

}

/***********************************************

	Update Window

************************************************/
void Scene::UpdateWindow()
{
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.20f, 0.20f, 0.20f, 1.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_COLOR_BUFFER_BIT);

		//Store new data in the texture
		UpdateTexture();
		//Render it
		renderShader.Use();
		RenderQuad();
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	//Generate Image when window is closed
	std::cout << "Saving Image into " << output_name << ".png" << std::endl;
	GenerateImage();
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
/***********************************************

	Parse a sphere

************************************************/
Sphere parse_sphere(const std::string * lines)
{
	std::string text_center = lines[0].substr(lines[0].find('(') + 1, lines[0].find(')') - lines[0].find('('));
	std::string text_radius = lines[0].substr(lines[0].find(')') + 1);
	std::string text_material = lines[1];

	std::string center_x = text_center.substr(0, text_center.find(','));
	text_center = text_center.substr(text_center.find(',') + 1);
	std::string center_y = text_center.substr(0, text_center.find(','));
	text_center = text_center.substr(text_center.find(',') + 1);
	std::string center_z = text_center.substr(0, text_center.find(')'));

	std::string color_r = text_material.substr(text_material.find('(') + 1, text_material.find(','));
	text_material = text_material.substr(text_material.find(',') + 1);
	std::string color_g = text_material.substr(0, text_material.find(','));
	text_material = text_material.substr(text_material.find(',') + 1);
	std::string color_b = text_material.substr(0, text_material.find(')'));

	text_material = text_material.substr(text_material.find(')') + 2);
	std::string spec_ref = text_material.substr(0, text_material.find(' '));
	text_material = text_material.substr(text_material.find(' ') + 1);
	std::string spec_exp = text_material.substr(0);

	vec3 center;
	float radius;
	vec3 color;
	float specular_reflection;
	float specular_exponent;

	center.x = std::stof(center_x);
	center.y = std::stof(center_y);
	center.z = std::stof(center_z);

	radius = std::stof(text_radius);

	color.r = std::stof(color_r);
	color.g = std::stof(color_g);
	color.b = std::stof(color_b);

	specular_reflection = std::stof(spec_ref);
	specular_exponent = std::stof(spec_exp);

	Material mat = parse_material(&lines[1]);

	Sphere sphere{ center,radius,mat };

	return sphere;
}
/***********************************************

	Parse a box

************************************************/
Box parse_box(const std::string * lines)
{
	std::string text_center = lines[0].substr(lines[0].find('(') + 1, lines[0].find(')') - lines[0].find('(') - 1);
	
	std::string text_vectors = lines[1];
	std::string text_length = text_vectors.substr(text_vectors.find_first_of('(') + 1, text_vectors.find_first_of(')') - 1);
	text_vectors = text_vectors.substr(text_vectors.find_first_of(' ') + 1);
	std::string text_width = text_vectors.substr(text_vectors.find_first_of('(') + 1, text_vectors.find_first_of(')') - 1);
	text_vectors = text_vectors.substr(text_vectors.find_first_of(' ') + 1);
	std::string text_height = text_vectors.substr(text_vectors.find_first_of('(') + 1, text_vectors.find_first_of(')') - 1);


	Material mat = parse_material(&lines[2]);

	vec3 center = extract_vec3(text_center);
	vec3 length = extract_vec3(text_length);
	vec3 width = extract_vec3(text_width);
	vec3 height = extract_vec3(text_height);


	return Box(center, length, width, height,mat);
}
/***********************************************

	Parse a light

************************************************/
Light parse_light(const std::string * lines)
{
	std::string text_components = lines[0];
	std::string text_position = text_components.substr(text_components.find_first_of('(') + 1, text_components.find_first_of(')') - text_components.find_first_of('(') - 1);
	text_components = text_components.substr(text_components.find_first_of(')') + 1);
	std::string text_color = text_components.substr(text_components.find_first_of('(') + 1, text_components.find_first_of(')') - text_components.find_first_of('(') - 1);
	text_components = text_components.substr(text_components.find_first_of(')') + 1);
	std::string text_radius = text_components;

	vec3 position = extract_vec3(text_position);
	vec3 color = extract_vec3(text_color);
	float radius = std::stof(text_radius);

	return Light(position,color,radius);
}
/***********************************************

	Parse the material

************************************************/
Material parse_material(const std::string * lines)
{
	std::string text_material = lines[0];
	std::string text_diffuse = text_material.substr(text_material.find_first_of('(') + 1, text_material.find_first_of(')') - 1);
	text_material = text_material.substr(text_material.find_first_of(')') + 2);
	std::string text_refle = text_material.substr(0, text_material.find_first_of(' '));
	text_material = text_material.substr(text_material.find_first_of(' ') + 1);
	std::string text_exp = text_material.substr(0, text_material.find_first_of(' '));
	text_material = text_material.substr(text_material.find_first_of(' ') + 1);
	std::string text_att = text_material.substr(text_material.find_first_of('(') + 1, text_material.find_first_of(')') - 1);
	text_material = text_material.substr(text_material.find_first_of(')') + 2);
	std::string text_elec_perm = text_material.substr(0, text_material.find_first_of(' '));
	text_material = text_material.substr(text_material.find_first_of(' ') + 1);
	std::string text_magn_perm = text_material.substr(0, text_material.find_first_of(' '));
	text_material = text_material.substr(text_material.find_first_of(' ') + 1);
	std::string text_roughness = text_material.substr(0, text_material.find_first_of(' '));

	vec3 diffuse = extract_vec3(text_diffuse);
	float refle = std::stof(text_refle);
	float exp = std::stof(text_exp);
	vec3 att = extract_vec3(text_att);
	float elec = std::stof(text_elec_perm);
	float mag = std::stof(text_magn_perm);
	float roug = std::stof(text_roughness);

	return Material{ diffuse,refle,exp,att,elec,mag,roug };
}
