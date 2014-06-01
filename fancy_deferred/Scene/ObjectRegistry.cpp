#include "ObjectRegistry.h"



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