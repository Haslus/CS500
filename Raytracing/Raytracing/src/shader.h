/**
* @file		shader.h
* @date 	02/08/2019
* @author	Asier Bilbao
* @par		Login: asier.b
* @par		Course: CS 562
* @par		Assignment #1
* @brief 	Framework
*
*/

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader
{
public:
	Shader() {};

	Shader(const std::string& vertex_path, const std::string& frag_path);
	void Use();

	void SetBool(const std::string& name, bool value);
	void SetInt(const std::string& name, int value);
	void SetFloat(const std::string& name, float value);
	void SetVec2(const std::string& name, vec2 value);
	void SetVec3(const std::string& name, vec3 value);
	void SetVec4(const std::string& name, vec4 value);
	void SetMat4(const std::string& name, mat4 mat);

	GLuint program;
private:
	void InitializeFragShader();
	void InitializeVertShader();
	void LinkProgram();

	void CheckCompileErrors(GLuint shader, std::string type);

	GLuint vertex_shader, fragment_shader;
	std::string vertex_shader_text, fragment_shader_text;

};