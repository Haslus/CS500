#pragma once
#include "pch.h"
#include "shader.h"

class CSGManager
{
public:
	enum CSGOperationType
	{
		Intersect,Union,Difference,
		SIntersect,SUnion,SDifference,
		Displacement, Twist, Bend,
		InfiniteRepetition
	};


	enum CSGShapeType
	{
		Sphere, Box, Torus, RoundBox, HexagonalPrism,
		TriangularPrism, Capsule, CappedCylinder, RoundedCylinder, CappedCone,
		Octahedron
	};

	struct Transform
	{
		vec3 m_position;
		vec3 m_scale;
		vec3 m_rotation;
	};

	
	struct CSGShape
	{
		CSGShapeType type;
		vec3 m_position;
		vec3 m_scale;
		vec3 m_rotation;
		glm::mat4 M2W;
		void calculateM2W();

	};

	struct CSGOperation
	{
		CSGOperationType type;
		int indexA;
		int indexB;
	};

	struct Scene
	{
		std::vector<CSGShape> shapes;
		std::vector<CSGOperation> operations;
	};

	std::vector<Scene> scenes;
	int current_scene = 0;
	std::vector<CSGShape> CSGshapes;
	std::vector<CSGOperation> CSGoperations;

	void SaveScene(const std::string & name);
	void LoadScene(const std::string & name);
	void LoadAllScenes();

	void Setup();

	void SetData(Shader& shader);

	bool noOperations = false;

};