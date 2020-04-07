#include "CSG.h"

std::vector<CSG::CSGShape> CSGshapes;
std::vector<CSG::CSGOperation> CSGoperations;

void CSG::Setup()
{
	CSGShape sphere{ Sphere ,vec3(0,0,0),vec3(1.2,1.2,1.2),vec3(0,0,0) };
	CSGShape box{ Box ,vec3(0,0,0),vec3(1,1,1),vec3(0,0,0) };
	CSGshapes.push_back(sphere);
	CSGshapes.push_back(box);
	CSGOperation op1{ Union,0,1 };
	CSGoperations.push_back(op1);
}

void CSG::SetData(Shader & shader)
{
	for (int i = 0; i < CSGshapes.size(); i++)
	{
		shader.SetInt("shapes[" + std::to_string(i) + "].type", CSGshapes[i].type);
		shader.SetVec3("shapes[" + std::to_string(i) + "].position", CSGshapes[i].m_position);
		shader.SetVec3("shapes[" + std::to_string(i) + "].scale", CSGshapes[i].m_scale);
		shader.SetVec3("shapes[" + std::to_string(i) + "].rotation", CSGshapes[i].m_rotation);
	}

	for (int i = 0; i < CSGoperations.size(); i++)
	{
		shader.SetInt("operations[" + std::to_string(i) + "].type", CSGoperations[i].type);
		shader.SetInt("operations[" + std::to_string(i) + "].indexA", CSGoperations[i].indexA);
		shader.SetInt("operations[" + std::to_string(i) + "].indexB", CSGoperations[i].indexB);
	//	shader.SetVec3("operations[" + std::to_string(i) + "].rotation", shapes[i].m_rotation);
	}
}
