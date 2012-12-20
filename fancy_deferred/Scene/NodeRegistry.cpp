#include "NodeRegistry.h"


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