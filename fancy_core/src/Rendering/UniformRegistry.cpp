#include "UniformRegistry.h"

UniformRegistry::UniformRegistry()
{

}

UniformRegistry::~UniformRegistry()
{

}

///Note: This function assumes it is not called with the same uniform twice
void UniformRegistry::RegisterUniformForSemantic( IUniform* pUniform, ShaderSemantics::Semantic eSemantic )
{
	std::map<ShaderSemantics::Semantic, std::vector<IUniform*>>::iterator iter = m_mapUniforms.find( eSemantic );

	//Semantic not yet registered
	if( iter == m_mapUniforms.end() )
	{
		std::vector<IUniform*> vUniforms;
		vUniforms.push_back( pUniform );
		m_mapUniforms[ eSemantic ] = vUniforms;
	}

	else
	{
		std::vector<IUniform*>& rvUniforms = iter->second;
		rvUniforms.push_back( pUniform );
	}		
}

void UniformRegistry::RemoveUniform( IUniform* pUniform )
{
	for( std::map<ShaderSemantics::Semantic, std::vector<IUniform*>>::iterator iter = m_mapUniforms.begin(); iter != m_mapUniforms.end(); iter++ )
	{
		std::vector<IUniform*>& rvUniforms = iter->second;
		std::vector<IUniform*>::iterator clUniformIter = std::find( rvUniforms.begin(), rvUniforms.end(), pUniform );

		//Uniform was found
		if( clUniformIter != rvUniforms.end() )
		{
			rvUniforms.erase( clUniformIter );
			return;
		}
	}
}

std::vector<IUniform*>* UniformRegistry::GetUniformsForSemantic( ShaderSemantics::Semantic eSemantic )
{
	std::map<ShaderSemantics::Semantic, std::vector<IUniform*>>::iterator iter = m_mapUniforms.find( eSemantic );

	if( iter != m_mapUniforms.end() )
	{
		//TODO:Unsure about this pointer
		return &iter->second;
	}

	return NULL;
}

bool UniformRegistry::GetUniformsForSemantic( ShaderSemantics::Semantic eSemantic, std::vector<IUniform*>& rVector )
{
	std::map<ShaderSemantics::Semantic, std::vector<IUniform*>>::iterator iter = m_mapUniforms.find( eSemantic );

	if( iter != m_mapUniforms.end() )
	{
		rVector = iter->second;
		return true;
	}

	return false;
}





