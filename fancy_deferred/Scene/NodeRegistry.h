#ifndef NODEREGISTRY_H
#define NODEREGISTRY_H

#include <Services/NameRegistry.h>

#include "SceneNode.h"

class DLLEXPORT NodeRegistry : public NameRegistry<SceneNode>
{
public:
	static NodeRegistry& getInstance(); 

private:
	NodeRegistry();
	virtual ~NodeRegistry();
};


#endif