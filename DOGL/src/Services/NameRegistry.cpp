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

ObjectRegistry::ObjectRegistry() : NameRegistry<BaseRenderableObject>()
{
	
}

ObjectRegistry::~ObjectRegistry()
{
	
}


////////////////////////////////////////////////////////
// NodeRegistry implementation
////////////////////////////////////////////////////////
NodeRegistry& NodeRegistry::getInstance()
{
	static NodeRegistry instance;
	
	return instance;
}

NodeRegistry::NodeRegistry() : NameRegistry<SceneNode>()
{
	
}


NodeRegistry::~NodeRegistry()
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


