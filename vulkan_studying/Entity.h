#pragma once
#include "Headers.h"

class Entity
{
public:
	virtual ~Entity(){};
	Entity(){};

	virtual void draw() = 0;
	virtual void update() = 0;
};

