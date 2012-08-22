#include "../includes.h"
#include "../Services/NameRegistry.h"


////////////////////////////////////////////////////////
// ObjectRegistry implementation
////////////////////////////////////////////////////////
ObjectRegistry& ObjectRegistry::getInstance()
{
	static ObjectRegistry instance;
	
	return instance;
}

ObjectRegistry::ObjectRegistry() : NameRegistry<Mesh>()
{
	
}

ObjectRegistry::~ObjectRegistry()
{
	
}


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


