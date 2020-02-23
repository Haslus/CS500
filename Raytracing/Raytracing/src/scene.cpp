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

			if (line.find("LIGHT") != std::string::npos)
			{
				lights.push_back(parse_light(&line));
			}

			if (line.find("AMBIENT") != std::string::npos)
			{
				std::string text_ambient = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
				global_ambient = extract_vec3(text_ambient);
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

			if (line.find("SAMPLES") != std::string::npos)
			{
				std::string samples = line.substr(line.find('=') + 1);
				this->samples = std::stoi(samples);
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
	GenerateRays();
	GenerateImage();
}
/***********************************************

	Throw a Ray

************************************************/
void Scene::Intersect(const Ray & ray)
{

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

	if (index == -1 || t == -1)
		intersection_data.push_back(vec3{ 0,0,0 });
	else
	{
		Material material = objects[index]->mat;
		vec3 normal = objects[index]->normal_at_intersection(ray, t);
		vec3 color = global_ambient * material.diffuse_color;
		
		//DEBUG
		intersection_data.push_back(material.diffuse_color);
		return;

		vec3 P = (ray.start + ray.dir * t);
		vec3 viewVec = glm::normalize(ray.start - P);

		vec3 ID{0,0,0};
		vec3 IS{ 0,0,0 };

		for (Light & light : lights)
		{
			//Calculate Diffuse + Specular
			vec3 lightDir = glm::normalize(light.position - P);

			float shadow_factor = 1;
			if (useHS)
			{
				//Calculate Hard Shadow
				Ray ray{ P + normal * epsilon, lightDir };
				bool HS_intersection = false;
				for (int i = 0; i < objects.size(); i++)
				{
					float d = objects[i]->intersection(ray);

					if (d != -1.0f)
					{
						HS_intersection = true;
						break;
					}

				}

				if(HS_intersection)
					shadow_factor = 0;
			}
			
			else if (useSS)
			{
				//Calculate Soft Shadows
				shadow_factor = 0;
				for (int s = 0; s < samples; s++)
				{
					Ray ray{ P + normal * epsilon, glm::normalize(light.bulb.get_random_point() - P) };

					for (int i = 0; i < objects.size(); i++)
					{
						float d = objects[i]->intersection(ray);

						if (d != -1.0f)
						{
							shadow_factor++;
							break;
						}

					}
				}

				shadow_factor = 1.0f - static_cast<float>(shadow_factor / samples);
			}

			

			//Calculate Diffuse factor
			ID += glm::max(glm::dot(normal, lightDir), 0.0f) * light.color * shadow_factor;

			//Calculate Specular factor
			vec3 reflection = glm::reflect(-lightDir, normal);
			IS += glm::pow(glm::max(glm::dot(reflection, viewVec), 0.0f), material.specular_exponent) * light.color * shadow_factor;

		}

		color += material.diffuse_color * ID + material.specular_reflection * IS;
		//Clamp color
		color = glm::clamp(color, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
		intersection_data.push_back(color);
	}

	
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
			Intersect(ray);

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
		converted_data.push_back(static_cast<unsigned char>(data.x * 255.99f));
		converted_data.push_back(static_cast<unsigned char>(data.y * 255.99f));
		converted_data.push_back(static_cast<unsigned char>(data.z * 255.99f));
	}
	stbi_write_png(output_name.c_str(), width, height, 3, converted_data.data(),0);
}

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


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

}
void Scene::UpdateWindow()
{
	
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
	Sphere sphere{ center,radius,color,specular_reflection,specular_exponent };

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
	//text_material = text_material.substr(text_material.find_first_of(' ') + 1);
	vec3 diffuse = extract_vec3(text_diffuse);
	float refle = std::stof(text_refle);
	float exp = std::stof(text_exp);
	vec3 att = extract_vec3(text_att);
	float elec = std::stof(text_elec_perm);
	float mag = std::stof(text_magn_perm);
	float roug = std::stof(text_roughness);

	return Material{ diffuse,refle,exp,att,elec,mag,roug };
}
