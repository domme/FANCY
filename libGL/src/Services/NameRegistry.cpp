#include "../includes.h"
#include "../Services/NameRegistry.h"


////////////////////////////////////////////////////////
// LightRegistry implementation
////////////////////////////////////////////////////////
LightRegistry& LightRegistry::getInstance()
{
	static LightRegistry instance;
	return instance;
}

LightRegistry::LightRegistry() : NameRegistry<Light>()
{

}

LightRegistry::~LightRegistry()
{

}


