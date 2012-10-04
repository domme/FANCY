#ifndef OBJECTREGISTRY_H
#define OBJECTREGISTRY_H

#include <Includes.h>
#include <Services/NameRegistry.h>

#include "BaseRenderableObject.h"


class ObjectRegistry : public NameRegistry<BaseRenderableObject>
{
public:
	static ObjectRegistry& getInstance();

private:
	ObjectRegistry();
	virtual ~ObjectRegistry();
};


#endif