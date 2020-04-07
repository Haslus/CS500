#pragma once
#include "pch.h"
#include "shader.h"

namespace CSG
{
	enum CSGOperationType
	{
		Union,Intersect,Difference,
		SUnion,SIntersect,SDifference
	};

	enum CSGDeformationType
	{
		Displacement,Twist,Bend
	};

	enum CSGShapeType
	{
		Sphere,Box,Torus,Link, Octahedron,Pyramid
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
	};

	struct CSGOperation
	{
		CSGOperationType type;
		int indexA;
		int indexB;
	};



	void Setup();

	void SetData(Shader& shader);

}