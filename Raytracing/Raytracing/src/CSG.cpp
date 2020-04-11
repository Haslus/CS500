#include "CSG.h"
# include <fstream>

vec3 extractVec3(std::string line)
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

void CSGManager::SaveScene(const std::string & name)
{
	std::ofstream file;
	remove(name.c_str());
	file.open(name);

	
	for (int i = 0; i < CSGshapes.size(); i++)
	{
		file << "Shape\n";
		file << std::to_string(CSGshapes[i].type) << '\n';
		file << std::to_string(CSGshapes[i].m_position.x) << ',' << std::to_string(CSGshapes[i].m_position.y) << ',' << std::to_string(CSGshapes[i].m_position.z) << '\n';
		file << std::to_string(CSGshapes[i].m_scale.x) << ',' << std::to_string(CSGshapes[i].m_scale.y) << ',' << std::to_string(CSGshapes[i].m_scale.z) << '\n';
		file << std::to_string(CSGshapes[i].m_rotation.x) << ',' << std::to_string(CSGshapes[i].m_rotation.y) << ',' << std::to_string(CSGshapes[i].m_rotation.z) << '\n';

	}

	
	for (int i = 0; i < CSGoperations.size(); i++)
	{
		file << "Operation\n";
		file << std::to_string(CSGoperations[i].type) << '\n';
		file << std::to_string(CSGoperations[i].indexA) << '\n';
		file << std::to_string(CSGoperations[i].indexB) << '\n';
	}

	file.close();
}

void CSGManager::LoadScene(const std::string & name)
{
	std::ifstream file;
	file.open(name);

	std::string line;

	if (file.is_open())
	{
		CSGshapes.clear();
		CSGoperations.clear();
		while (getline(file, line))
		{
			if (line.find("Shape") != std::string::npos)
			{
				CSGShape shape;
				getline(file, line);
				shape.type = static_cast<CSGShapeType>(std::stoi(line));
				getline(file, line);
				shape.m_position = extractVec3(line);
				getline(file, line);
				shape.m_scale = extractVec3(line);
				getline(file, line);
				shape.m_rotation = extractVec3(line);
				CSGshapes.push_back(shape);
				
			}
			else if (line.find("Operation") != std::string::npos)
			{
				CSGOperation op;
				getline(file, line);
				op.type = static_cast<CSGOperationType>(std::stoi(line));
				getline(file, line);
				op.indexA = std::stoi(line);
				getline(file, line);
				op.indexB = std::stoi(line);
				CSGoperations.push_back(op);
			}
			
		}

		file.close();
	}

}

void CSGManager::Setup()
{
	//CSGShape sphere{ Sphere ,vec3(0,0,0),vec3(1.2,1.2,1.2),vec3(0,0,0) };
	//CSGShape box{ Box ,vec3(0,0,0),vec3(1,1,1),vec3(0,0,0) };
	//CSGshapes.push_back(sphere);
	//CSGshapes.push_back(box);

	//CSGShape sphere1{ Sphere ,vec3(0,0,0),vec3(1.2,1.2,1.2),vec3(0,0,0) };
	//CSGShape sphere2{ Sphere ,vec3(2,0,0),vec3(1.2,1.2,1.2),vec3(0,0,0) };
	//CSGShape sphere3{ Sphere ,vec3(0,2,0),vec3(1.2,1.2,1.2),vec3(0,0,0) };
	//CSGshapes.push_back(sphere1);
	//CSGshapes.push_back(sphere2);
	//CSGshapes.push_back(sphere3);
	//
	//CSGOperation op1{ Difference,0,1 };
	//CSGoperations.push_back(op1);
	//CSGOperation op2{ Difference,-1,2 };
	//CSGoperations.push_back(op2);
}

void CSGManager::SetData(Shader & shader)
{
	for (int i = 0; i < CSGshapes.size(); i++)
	{
		shader.SetInt("shapes[" + std::to_string(i) + "].type", CSGshapes[i].type);
		shader.SetVec3("shapes[" + std::to_string(i) + "].position", CSGshapes[i].m_position);
		shader.SetVec3("shapes[" + std::to_string(i) + "].scale", CSGshapes[i].m_scale);
		shader.SetVec3("shapes[" + std::to_string(i) + "].rotation", CSGshapes[i].m_rotation);
		//shader.SetMat4("shapes[" + std::to_string(i) + "].M2W", CSGshapes[i].M2W);
	}

	shader.SetInt("shapeCount", CSGshapes.size());

	shader.SetBool("DoOperations", noOperations);

	if (noOperations)
	{
		for (int i = 0; i < CSGoperations.size(); i++)
		{
			shader.SetInt("operations[" + std::to_string(i) + "].type", CSGoperations[i].type);
			shader.SetInt("operations[" + std::to_string(i) + "].indexA", CSGoperations[i].indexA);
			shader.SetInt("operations[" + std::to_string(i) + "].indexB", CSGoperations[i].indexB);
			//	shader.SetVec3("operations[" + std::to_string(i) + "].rotation", shapes[i].m_rotation);
		}

		shader.SetInt("opCount", CSGoperations.size());
	}
	
}

void CSGManager::CSGShape::calculateM2W()
{
	mat4 T = glm::translate(this->m_position);

	mat4 RX = glm::rotate(m_rotation.x, vec3(1, 0, 0));
	mat4 RY = glm::rotate(m_rotation.y, vec3(0, 1, 0));
	mat4 RZ = glm::rotate(m_rotation.z, vec3(0, 0, 1));

	M2W = T * RX * RY * RZ;


}
