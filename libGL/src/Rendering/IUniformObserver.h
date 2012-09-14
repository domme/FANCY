#ifndef UNIFORMOBSERVER_H
#define UNIFORMOBSERVER_H

#include "IUniform.h"

class DLLEXPORT IUniformObserver
{
public:
	 virtual void UniformChanged( IUniform* pUniform ) = 0;
	 virtual void CleanUniforms() = 0;
};

#endif 