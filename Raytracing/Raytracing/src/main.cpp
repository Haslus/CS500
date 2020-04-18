/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_1
Author: Asier Bilbao / asier.b
Creation date:1/8/2020
----------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include <iostream>
#include "scene.h"

int main(int argc, char *argv[])
{
	Scene scene;

	//std::cout << "Generating Image of size " << scene.width << "x" << scene.height << " from " << scene.input_name << std::endl;
	scene.InitializeWindow();
	scene.UpdateWindow();
	return 0;
}