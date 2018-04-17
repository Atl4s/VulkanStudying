#pragma once
#include "Entity.h"
#include "Cleaner.h"
#include <vulkan/vulkan.h>

//SSBO struct
struct FluidParticle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 force;
	float density;
	float pressure;
	uint16_t fluidIndex = 0;//overwrites in Fluid constructor
};

//UBO struct
struct FluidParams
{
	uint64_t particlesCount;
	float particleRadius;
	float particleMass;
	float particleRestingDensity;
	float particleStiffness;
	float particleViscosity;
	float smoothingLength;
	glm::vec3 force;
	float timeStep;
	uint16_t fluidIndex = 0;//overwrites in Fluid constructor
};

class Fluid :
	public Entity
{
public:
	Fluid();
	Fluid(FluidParticle, FluidParams);
	~Fluid();
private:
	 FluidParticle m_fluidParticle;
	 FluidParams m_fluidParams;

	uint16_t m_fluidIndex;
};

