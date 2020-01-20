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

Scene::Scene(const std::string & filepath, int width, int height, std::string output_name)
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
			
			std::string text_center = line.substr(line.find('(') + 1, line.find(')') - line.find('('));
			std::string text_radius = line.substr(line.find(')') + 1);
			std::getline(file, line);
			std::string text_color = line;

			std::string center_x = text_center.substr(0, text_center.find(','));
			text_center = text_center.substr(text_center.find(',') + 1);
			std::string center_y = text_center.substr(0, text_center.find(','));
			text_center = text_center.substr(text_center.find(',') + 1);
			std::string center_z = text_center.substr(0, text_center.find(')'));

			std::string color_r = text_color.substr(text_color.find('(') + 1, text_color.find(','));
			text_color = text_color.substr(text_color.find(',') + 1);
			std::string color_g = text_color.substr(0, text_color.find(','));
			text_color = text_color.substr(text_color.find(',') + 1);
			std::string color_b = text_color.substr(0, text_color.find(')'));

			vec3 center;
			float radius;
			vec3 color;

			center.x = std::stof(center_x);
			center.y = std::stof(center_y);
			center.z = std::stof(center_z);

			radius = std::stof(text_radius);

			color.r = std::stof(color_r);
			color.g = std::stof(color_g);
			color.b = std::stof(color_b);

			Sphere sphere{center,radius,color};

			spheres.push_back(sphere);
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

			vec3 center = extract_vec3(text_center);
			vec3 up = extract_vec3(text_up);
			vec3 right = extract_vec3(text_right);
			float eye = std::stof(text_eye);

			vec3 forward = glm::normalize(glm::cross(up,right));
			vec3 start = center - forward * eye;

			float halfWidth  = static_cast<float>(width / 2);
			float halfHeigth = static_cast<float>(height / 2);

			for( int i = 0; i < height; i++)
				for (int j = 0; j < width; j++)
				{
					vec3 P = center + static_cast<float>((j - halfWidth + 0.5f) / halfWidth) * right - static_cast<float>((i - halfHeigth + 0.5f) / halfHeigth) * up;
					Ray ray{ start,P - start};
					rays.push_back(ray);
					Intersect(ray);

				}


		}
	}

	file.close();
	name = output_name;
	GenerateImage();
}

void Scene::Intersect(const Ray & ray)
{

	float d_max = FLT_MAX - 1.f;
	int index = -1;
	for (int i = 0; i < spheres.size(); i++)
	{
		float d = intersection_ray_sphere(ray, spheres[i]);

		if (d >= 0.f && d <= d_max)
		{
			d_max = d;
			index = i;
		}

	}

	if (index == -1)
		intersection_data.push_back(vec3{ 0,0,0 });
	else
		intersection_data.push_back(spheres[index].color);

	
}


void Scene::GenerateImage()
{
	std::vector<unsigned char> converted_data;
	for (auto data : intersection_data)
	{
		converted_data.push_back(static_cast<unsigned char>(data.x * 255.99f));
		converted_data.push_back(static_cast<unsigned char>(data.y * 255.99f));
		converted_data.push_back(static_cast<unsigned char>(data.z * 255.99f));
	}
	stbi_write_png(name.c_str(), width, height, 3, converted_data.data(),0);
}
