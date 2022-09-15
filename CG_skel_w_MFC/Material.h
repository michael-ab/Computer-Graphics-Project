#pragma once

#include "scolor.h"

class Material
{
public:
	Material():
		ambient(0.2,0.2,0.2),
		diffuse(0.7,0.7,0.7),
		emissive(0,0,0),
		specular(1,1,1),
		shininess(4)
		{}
	scolor ambient;
	scolor diffuse;
	scolor emissive;
	scolor specular;
	int shininess;

};

