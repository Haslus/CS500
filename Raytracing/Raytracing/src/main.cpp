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

	if (argc == 1)
	{
		std::cout << "No argument, using default size (500 x 500)." << std::endl;
		std::cout << "No argument, using default input file (SampleScene01.txt)." << std::endl;
		std::cout << "No argument, using default output file name (Out.png)." << std::endl;
		scene = { "SampleScene02.txt",500,500 };
	
	}
	else if (argc == 3)
	{
		std::cout << "No argument, using default input file (SampleScene01.txt)." << std::endl;
		std::cout << "No argument, using default output file name (Out.png)." << std::endl;
		scene = { "SampleScene02.txt", std::stoi(argv[1]) ,std::stoi(argv[2]) };
	}
	else if (argc == 4)
	{
		
		std::cout << "No argument, using default output file name (Out.png)." << std::endl;
		scene = { argv[3], std::stoi(argv[1]) ,std::stoi(argv[2]) };
	}
	else if (argc == 5)
	{
		scene = { argv[3],std::stoi(argv[1]) ,std::stoi(argv[2]), argv[4] };
	}

	scene.Setup();

	return 0;
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
