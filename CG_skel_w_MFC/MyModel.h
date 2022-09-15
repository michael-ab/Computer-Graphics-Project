#pragma once
#include <vector>
#include "scolor.h"


using namespace std;

class MyModel
{
public:
	virtual vec3 defaultLoc() = 0;
	virtual ~MyModel() {}
};

