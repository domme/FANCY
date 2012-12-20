#ifndef UNIFORMREGISTRY_H
#define UNIFORMREGISTRY_H

#include "../includes.h"
#include "ShaderSemantics.h"
#include "IUniform.h"

class DLLEXPORT UniformRegistry
{
public:
	static UniformRegistry& GetInstance()
	{
		static UniformRegistry instance;

		return instance;
	}

	void RegisterUniformForSemantic( IUniform* pUniform, ShaderSemantics::Semantic eSemantic );
	void RemoveUniform( IUniform* pUniform );
	
	//void SetDirtyForSemantic( ShaderSemantics::Semantic eSemantic );
	//void SetDirtyForSemantics( ShaderSemantics::Semantic* eSemantic, uint uNumSemantics );
	
	std::map<ShaderSemantics::Semantic, std::vector<IUniform*>>&		GetUniformMap() { return m_mapUniforms; }
	 std::vector<IUniform*>*	GetUniformsForSemantic( ShaderSemantics::Semantic eSemantic ); 
	bool				GetUniformsForSemantic( ShaderSemantics::Semantic eSemantic, std::vector<IUniform*>& rVector ); 

private:
	UniformRegistry();
	~UniformRegistry();

	
	
	std::map<ShaderSemantics::Semantic, std::vector<IUniform*>>  m_mapUniforms;

};


#endif