#pragma once

#include "vec.h"
#include "scolor.h"

enum e_lightSource 
{ 
	POINT_S, 
	PARALLEL_S, 
	AREA_S
};
enum e_lightFormat 
{ 
	AMBIENT_L, 
	REGULAR_L
};

class Light
{
public:
	Light(e_lightFormat _lightType, e_lightSource _lightSource, vec4 _location, scolor _lightColor, vec4 _direction);
	Light(const Light& l);
	~Light(void);
	e_lightFormat lightFormat;
	e_lightSource lightSource;
	scolor color;
	vec4 direction;
	vec4 location;


};
