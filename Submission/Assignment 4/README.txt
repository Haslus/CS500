/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_asier.b_4
Author: Asier Bilbao / asier.b
Creation date: 03/19/2020
----------------------------------------------------------------------------------------------------------*/
• How to run your program: Visual Studio 2017 x64
• How to use your program: First argument is width, second is heigth, third is the input file, and fourth is output file
You can change some parameters on the config file. I think each value is self-explanatory, but just in case:
So for Shadows your options are NONE,HARD and SOFT
For AntiAliasing you have NONE, SUPER (supersampling), ADAPTIVE (supersampling adaptive)
AA_SAMPLES is the number of smaples for SUPER
ADAPTIVE_DEPTH is the depth of the recursion for ADAPTIVE
PREVIEW switches between render on the window or not (true/false)

• Important parts of the code: Collision and Scene has the important stuff.
The function Raycast is the one doing all the magic
• Known issues and problems: Some of your scenes.txt had an extra # so I couldn't parse it correctly. Be careful with that.