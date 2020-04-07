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

vec2 offset;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void*)window;
	offset = { xoffset,yoffset };
}

bool updateShader = false;


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
	//Initialize just in case
	air.attenuation = vec3(1, 1, 1);
	air.magnetic_permeability = 1;
	air.electric_perimittivity = 1;
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
				std::string lines[2] = { line,line2 };
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

						indices.push_back(vec3{ face_1.x - 1,face_2.x - 1, face_3.x  - 1});
					
					}

				}

				std::getline(file, line);
				std::string text_pos = line.substr(1, line.find_first_of(')') - 1);
				line = line.substr(line.find_first_of(' ') + 1);
				std::string text_angle = line.substr(1, line.find_first_of(')') - 1);
				line = line.substr(line.find_first_of(' ') + 1);
				std::string text_scale = line;

				std::getline(file, line);

				Mesh * mesh = new Mesh(extract_vec3(text_pos),extract_vec3(text_angle),std::stof(text_scale),
					vertices,indices,parse_material(&line));


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

			//if (line.find("CAMERA") != std::string::npos)
			//{
			//	std::string text_center = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
			//	line = line.substr(line.find(')') + 1);
			//	std::string text_right = line.substr(2, line.find(')') - 2);
			//	line = line.substr(line.find(')') + 1);
			//	std::string text_up = line.substr(2, line.find(')') - 2);
			//	line = line.substr(line.find(')') + 1);
			//	std::string text_eye = line.substr(0);
			//
			//	camera.center = extract_vec3(text_center);
			//	camera.up = extract_vec3(text_up);
			//	camera.right = extract_vec3(text_right);
			//	camera.eye = std::stof(text_eye);
			//
			//
			//
			//
			//}


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

			if (line.find("SHADOWS") != std::string::npos)
			{
				if (line.find("HARD") != std::string::npos)
				{
					S_method = Shadows::HARD;
				}
				else if (line.find("SOFT") != std::string::npos)
				{
					S_method = Shadows::SOFT;
				}
				else
				{
					S_method = Shadows::NONE;
				}
			}

			if (line.find("AA_METHOD") != std::string::npos)
			{
				if (line.find("SUPER") != std::string::npos)
				{
					AA_method = AntiAliasing::SUPER;
				}
				else if (line.find("ADAPTIVE") != std::string::npos)
				{
					AA_method = AntiAliasing::ADAPTIVE;
				}
				else
				{
					AA_method = AntiAliasing::NONE;
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

			if (line.find("AA_SAMPLES") != std::string::npos)
			{
				std::string AA_samples = line.substr(line.find('=') + 1);
				this->AA_samples = std::stoi(AA_samples);
			}

			if (line.find("ADAPTIVE_DEPTH") != std::string::npos)
			{
				std::string A_max_depth = line.substr(line.find('=') + 1);
				this->adaptive_AA_recursion = std::stoi(A_max_depth);
			}

			if (line.find("PREVIEW") != std::string::npos)
			{
				if (line.find("true") != std::string::npos)
				{
					preview = true;
				}
				else
				{
					preview = false;
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

	Light l;
	l.position = vec3(4, 2, -4);
	l.color = vec3(0.6);
	l.radius = 0.1;
	lights.push_back(l);
}
/***********************************************

	Throw a Ray

************************************************/
vec3 Scene::Raycast(const Ray & ray, const int& d, const bool& transmitting)
{
	//MAX DEPTH REACHED
	if (d == 0)
		return vec3{ 0, 0, 0 };

	float d_max = FLT_MAX - 1.f;
	int index = -1;
	float t = -1.0f;
	//Intersect objects
	IntersectionData data;
	for (int i = 0; i < objects.size(); i++)
	{

		IntersectionData temp = objects[i]->intersection_data(ray);

		if (temp.t > 0.f && temp.t < d_max)
		{
			d_max = temp.t;
			index = i;
			data = temp;
		}
		

	}
	//NO INTERSECTION
	if (index == -1 || data.t == -1.0f)
	{

		return vec3{ 0,0,0 };
	}

	Material incident, transmit;
	Material material = objects[index]->mat;

	//Switch material depending on enter/exit
	if (transmitting == false)
	{
		incident = air;
		transmit = material;
	}
	else
	{
		incident = material;
		transmit = air;
	}

	vec3 normal = data.normal;
	vec3 color = global_ambient * material.diffuse_color;

	//Inverse normal if exiting object
	if (glm::dot(ray.dir, normal) > 0.0f)
	{
		normal = -normal;
	}

	vec3 P = data.PI + epsilon * normal;
	vec3 viewVec = glm::normalize(ray.start - P);


	vec3 ID{ 0, 0, 0 };
	vec3 IS{ 0, 0, 0 };

	float n_i = glm::sqrt(incident.electric_perimittivity * incident.magnetic_permeability);
	float n_t = glm::sqrt(transmit.electric_perimittivity * transmit.magnetic_permeability); //Index of refraction of object at P

	//Reflection coefficient
	float n_ratio = (n_i / n_t);
	float real_R;
	if (n_ratio < 0.01f)
	{
		real_R = 1.0f;
	}
	else
	{
		
		float cos_I = -glm::dot(glm::normalize(ray.dir), normal);

		float magnetic_ratio = (incident.magnetic_permeability / transmit.magnetic_permeability);

		float radicant = 1.0f - (glm::pow(n_ratio, 2) * (1.0f - glm::pow(cos_I, 2)));

		if (radicant < 0.0f)
		{
			real_R = 1.0f;
		}
		else
		{
			radicant = glm::sqrt(radicant);

			float ER_perp = n_ratio * cos_I - magnetic_ratio * radicant;

			float EI_perp = n_ratio * cos_I + magnetic_ratio * radicant;

			float ER_para = magnetic_ratio * cos_I - n_ratio * radicant;

			float EI_para = magnetic_ratio * cos_I + n_ratio * radicant;

			real_R = 0.5f * (glm::pow(ER_perp / EI_perp, 2) + glm::pow(ER_para / EI_para, 2));
		}

	}

	
	//Calculate coefficients
	float ks = material.specular_reflection;
	float R = real_R * ks;
	float real_T = 1.0f - real_R;
	float T = real_T * ks;
	float Absorb = 1.0f - T - R;
	

	//APPLY LOCAL ILLUMINATION
	for (Light & light : lights)
	{
		//Calculate Diffuse + Specular
		vec3 lightDir = glm::normalize(light.position - P);

		float shadow_factor = 1;

		if (S_method == Shadows::HARD)
		{
			//Calculate Hard Shadow
			Ray temp_ray{ P, lightDir };
			float distance = glm::length(light.position - P);

			for (int i = 0; i < objects.size(); i++)
			{
				IntersectionData temp = objects[i]->intersection_data(temp_ray);

				if (temp.t != -1.0f && temp.t <= distance)
				{
					vec3 point = temp.PI;

					if (glm::length(point - P) <= distance)
					{
						shadow_factor = 0;
						break;

					}
				}

			}

		}

		else if (S_method == Shadows::SOFT)
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
					IntersectionData temp = objects[i]->intersection_data(ray);

					if (temp.t != -1.0f && temp.t <= distance)
					{
						vec3 point = temp.PI;

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

	color += material.diffuse_color * ID + ks * IS;
	color *= Absorb;

	//Use attenuation
	vec3 att = { 1.0f,1.0f,1.0f };

	if (use_attenuation)
	{
		att.x = glm::pow(incident.attenuation.x, glm::length(ray.start - P));
		att.y = glm::pow(incident.attenuation.y, glm::length(ray.start - P));
		att.z = glm::pow(incident.attenuation.z, glm::length(ray.start - P));
	}

	if (R > 0.0f)
	{
		vec3 reflected_dir = glm::normalize(glm::reflect(glm::normalize(ray.dir), normal));

		if (material.roughness > 0.0f)
		{
			vec3 reflec_color = vec3(0, 0, 0);

			for (int i = 0; i < reflection_samples; i++)
			{
				Ray reflected_ray{ P, glm::normalize(reflected_dir + sample_sphere(material.roughness)) };
				reflec_color += Raycast(reflected_ray, d - 1, transmitting);
			}

			reflec_color /= reflection_samples;
			color = (color + R * reflec_color);
		}
		else
		{

			Ray reflected_ray{ data.PI + normal * epsilon, reflected_dir };
			color = (color + R * Raycast(reflected_ray, d - 1, transmitting));
			

		}
	}

	//Transmission/Refraction
	if (T > 0.0f)
	{
		vec3 transmitted_dir = glm::refract(glm::normalize(ray.dir), normal, n_ratio);
		Ray transmitted_ray{ data.PI - normal * epsilon, transmitted_dir };
		color = (color + T * Raycast(transmitted_ray, d - 1, !transmitting));

	}
	
	//Absorb

	return att * color;



}
/***********************************************

	Generate Rays

************************************************/
void Scene::GenerateRays()
{
#ifdef RAYCASTING
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
			intersection_data[i * width + j] = Raycast(ray, max_depth, false);

		}
#endif // 

	
}
/***********************************************

	Generate Rays from BEGIN height to END heigth

************************************************/
void Scene::GenerateRaysRange(int begin, int end)
{
#ifdef RAYCASTING
	vec3 forward = glm::normalize(glm::cross(camera.up, camera.right));
	vec3 start = camera.center - forward * camera.eye;

	float halfWidth = static_cast<float>(width / 2);
	float halfHeigth = static_cast<float>(height / 2);

	//Generate rays
	for (int i = begin; i < end; i++)
		for (int j = 0; j < width; j++)
		{
			vec3 P = camera.center + static_cast<float>((j - halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i - halfHeigth + 0.5f) / halfHeigth) * camera.up;
			

			switch (AA_method)
			{
			case AntiAliasing::NONE:
			{
				Ray ray{ start,glm::normalize(P - start) };
				intersection_data[i * width + j] = glm::clamp(Raycast(ray, max_depth, false), vec3(0), vec3(1));
				break;
			}
			case AntiAliasing::SUPER:
			{
				int size = glm::sqrt(AA_samples);

				vec3 right_P = camera.center + static_cast<float>((j + 1- halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i - halfHeigth + 0.5f) / halfHeigth) * camera.up;
				vec3 delta_right = right_P - P;
				delta_right /= size;
				vec3 down_P = camera.center + static_cast<float>((j - halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i + 1 - halfHeigth + 0.5f) / halfHeigth) * camera.up;
				vec3 delta_down = down_P - P;
				delta_down /= size;

				vec3 color = vec3(0);
				bool odd_size = size % 2 == 0 ? false : true;

				size /= 2;

				for (float d = -size; d <= size; d++)
				{
					if (!odd_size && d == 0)
						continue;

					for (float r = -size; r <= size; r++)
					{
						if (!odd_size && r == 0)
							continue;

						Ray ray{ start,glm::normalize(P + delta_right * r + delta_down * d - start) };
						color += glm::clamp(Raycast(ray, max_depth, false), vec3(0), vec3(1));
					}
				}

				color /= AA_samples;
				intersection_data[i * width + j] = color;
				break;
			}
			case AntiAliasing::ADAPTIVE:
			{
				//int size = glm::sqrt(AA_samples);

				vec3 right_P = camera.center + static_cast<float>((j + 1 - halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i - halfHeigth + 0.5f) / halfHeigth) * camera.up;
				vec3 delta_right = right_P - P;
				delta_right /= 2;
				vec3 down_P = camera.center + static_cast<float>((j - halfWidth + 0.5f) / halfWidth) * camera.right - static_cast<float>((i + 1 - halfHeigth + 0.5f) / halfHeigth) * camera.up;
				vec3 delta_down = down_P - P;
				delta_down /= 2;

				vec3 left_top = P - delta_right - delta_down;
				vec3 right_top = P + delta_right - delta_down;
				vec3 left_bot = P - delta_right + delta_down;
				vec3 right_bot = P + delta_right + delta_down;

				vec3 corners[4] = { left_top ,right_top ,left_bot,right_bot };
				vec3 colors[5] = {vec3(0)};
				for (int k = 0; k < 4; k++)
				{
					Ray ray{ start, glm::normalize(corners[k] - start) };
					colors[k] = glm::clamp(Raycast(ray, max_depth, false), vec3(0), vec3(1));
				}

				colors[4] = colors[0];

				bool tolerated = true;
				for (int k = 0; k < 4; k++)
				{
					float tolerance = glm::length(colors[k] - colors[k + 1]);

					if (tolerance > adaptive_AA_tolerance)
					{
						tolerated = false;
						break;
					}

				}

				if (tolerated)
				{
					vec3 final_color = colors[0] + colors[1] + colors[2] + colors[3];
					final_color /= 4.0f;
					intersection_data[i * width + j] = final_color;
				}
				//Need to subdivide
				else
				{
					vec3 left_top_color = AdpativeASubdivision(start,left_top, delta_right / 2.0f, delta_down / 2.0f, adaptive_AA_recursion - 1);
					vec3 right_top_color = AdpativeASubdivision(start,left_top + delta_right / 2.0f, delta_right / 2.0f, delta_down / 2.0f, adaptive_AA_recursion - 1);
					vec3 left_bot_color = AdpativeASubdivision(start,left_top + delta_down / 2.0f, delta_right / 2.0f, delta_down / 2.0f, adaptive_AA_recursion - 1);
					vec3 right_bot_color = AdpativeASubdivision(start,left_top + delta_right / 2.0f + delta_down / 2.0f, delta_right / 2.0f, delta_down / 2.0f, adaptive_AA_recursion - 1);

					vec3 final_color = left_top_color + right_top_color + left_bot_color + right_bot_color;
					final_color /= 4.0f;
					intersection_data[i * width + j] = final_color;
				}

				break;
			}
			}
			

		}
#endif
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

vec3 Scene::AdpativeASubdivision(vec3 start, vec3 P, vec3 delta_right, vec3 delta_down, int recursion)
{

	//Get the four corners

	vec3 left_top = P;
	vec3 right_top = P + delta_right - delta_down;
	vec3 left_bot = P - delta_right + delta_down;
	vec3 right_bot = P + delta_right + delta_down;

	vec3 corners[4] = { left_top ,right_top ,left_bot,right_bot };
	vec3 colors[5];
	for (int k = 0; k < 4; k++)
	{
		Ray ray{ start, glm::normalize(corners[k] - start) };
		colors[k] = glm::clamp(Raycast(ray, max_depth, false), vec3(0), vec3(1));
	}

	colors[4] = colors[0];

	bool tolerated = true;
	for (int k = 0; k < 4; k++)
	{
		float tolerance = glm::length(colors[k] - colors[k + 1]);

		if (tolerance > adaptive_AA_tolerance)
		{
			tolerated = false;
			break;
		}

	}

	if (tolerated || recursion == 0)
	{
		vec3 final_color = colors[0] + colors[1] + colors[2] + colors[3];
		final_color /= 4.0f;
		return final_color;
	}
	//Need to subdivide
	else
	{
		vec3 left_top_color = AdpativeASubdivision(start,left_top, delta_right / 2.0f, delta_down / 2.0f, recursion - 1);
		vec3 right_top_color = AdpativeASubdivision(start,left_top + delta_right / 2.0f, delta_right / 2.0f, delta_down / 2.0f, recursion - 1);
		vec3 left_bot_color = AdpativeASubdivision(start,left_top + delta_down / 2.0f, delta_right / 2.0f, delta_down / 2.0f, recursion - 1);
		vec3 right_bot_color = AdpativeASubdivision(start,left_top + delta_right / 2.0f + delta_down / 2.0f, delta_right / 2.0f, delta_down / 2.0f, recursion - 1);

		vec3 final_color = left_top_color + right_top_color + left_bot_color + right_bot_color;
		final_color /= 4.0f;
		return final_color;
	}
}

void Scene::GenerateRaysCSG(int begin, int end)
{
#ifdef RAYCASTING



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
			intersection_data[i * width + j] = glm::clamp(Raymarch(ray), vec3(0), vec3(1));

		}

#endif // RAYCASTING	
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

	//int division = width / threads;
	//
	//for (int i = 0; i < threads; i++)
	//{
	//	if (i == threads - 1)
	//	{
	//		std::thread ray1(&Scene::GenerateRaysCSG, this, division * i, width);
	//		ray1.detach();
	//	}
	//	else
	//	{
	//		std::thread ray1(&Scene::GenerateRaysCSG,this, division * i, division * (i + 1));
	//		ray1.detach();
	//	}
	//}

	
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	renderShader = Shader{ "shaders/normal.vert","shaders/normal.frag" };

	glfwSetScrollCallback(window, scroll_callback);
	

}

/***********************************************

	Update Window

************************************************/
void Scene::UpdateWindow()
{
	CSG::Setup();
	
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_COLOR_BUFFER_BIT);

		//Store new data in the texture
		//UpdateTexture();
		//Render it
		renderShader.Use();
		CSG::SetData(renderShader);
		renderShader.SetVec2("Resolution", vec2(width, height));
		renderShader.SetFloat("Time", glfwGetTime());
		renderShader.SetVec3("camEye", m_cam.camPos);
		renderShader.SetVec3("camFront", m_cam.camFront);
		RenderQuad();

		if (updateShader)
		{
			renderShader = Shader{ "shaders/normal.vert","shaders/normal.frag" };
			updateShader = false;
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	
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

	if (glfwGetKey(window, GLFW_KEY_F5)) {
		renderShader = Shader{ "shaders/normal.vert","shaders/normal.frag" };
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
			float speed = 1.0f;


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
	float m_radius;

	center.x = std::stof(center_x);
	center.y = std::stof(center_y);
	center.z = std::stof(center_z);

	m_radius = std::stof(text_radius);


	Material mat = parse_material(&lines[1]);

	Sphere sphere{ center,m_radius,mat };

	return sphere;
}
/***********************************************

	Parse a box

************************************************/
Box parse_box(const std::string * lines)
{
	std::string text_center = lines[0].substr(lines[0].find('(') + 1, lines[0].find(')') - lines[0].find('(') - 1);
	
	std::string text_vectors = lines[0].substr(lines[0].find(')') + 2);
	std::string text_length = text_vectors.substr(text_vectors.find_first_of('(') + 1, text_vectors.find_first_of(')') - 1);
	text_vectors = text_vectors.substr(text_vectors.find_first_of(' ') + 1);
	std::string text_width = text_vectors.substr(text_vectors.find_first_of('(') + 1, text_vectors.find_first_of(')') - 1);
	text_vectors = text_vectors.substr(text_vectors.find_first_of(' ') + 1);
	std::string text_height = text_vectors.substr(text_vectors.find_first_of('(') + 1, text_vectors.find_first_of(')') - 1);


	Material mat = parse_material(&lines[1]);

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
	float m_radius = std::stof(text_radius);

	return Light(position,color,m_radius);
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
