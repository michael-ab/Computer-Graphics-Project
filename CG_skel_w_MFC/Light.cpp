#include "StdAfx.h"
#include "Light.h"


Light::Light(e_lightFormat _lightType, e_lightSource _lightSource, vec4 _location, scolor _lightColor, vec4 _direction) :
	lightFormat(_lightType), lightSource(_lightSource), location(_location), color(_lightColor), direction(_direction)
{
}

Light::Light(const Light& lightIn)
{
	lightFormat = lightIn.lightFormat;
	color = lightIn.color;
	lightSource = lightIn.lightSource;
	location = lightIn.location;
	direction = lightIn.direction;
}


Light::~Light(void)
{
}


