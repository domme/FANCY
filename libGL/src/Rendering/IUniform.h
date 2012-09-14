#ifndef IUNIFORM_H
#define IUNIFORM_H

#include "../includes.h"
#include "ShaderSemantics.h"


class IUniformObserver;

class DLLEXPORT IUniform
{
public:
	virtual ~IUniform() {};
	virtual void Upload() const = 0;
	virtual IUniform* Clone() const = 0;
	virtual void SetObserver( IUniformObserver* pObserver ) = 0;
	virtual IUniform* SetSemantic( ShaderSemantics::Semantic eSemantic ) = 0;
	virtual ShaderSemantics::Semantic GetSemantic() const = 0;
	virtual GLint GetGLhandle() const  = 0;
	virtual int GetIndex() const = 0;
	virtual void SetIndex( int iN ) = 0;
	virtual const String& GetName() const = 0;
	
};

#endif