#include "Fluid.h"


Fluid::Fluid()
{
	static uint16_t id = 0;
	m_fluidIndex = id++;
}


Fluid::Fluid(FluidParticle fluidParticle, FluidParams fluidParams) : 
m_fluidParticle(fluidParticle),
m_fluidParams(fluidParams)
{
	static uint16_t id = 0;
	m_fluidIndex = id++;
	m_fluidParticle.fluidIndex = m_fluidIndex;
	m_fluidParams.fluidIndex = m_fluidIndex;
}

Fluid::~Fluid()
{
}
