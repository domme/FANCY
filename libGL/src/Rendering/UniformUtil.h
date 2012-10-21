#ifndef UNIFORMUTIL_H
#define UNIFORMUTIL_H

#include "../Includes.h"
#include "IUniform.h"

class  DLLEXPORT UniformUtil
{
public:
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::mat4& value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::mat3& value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::vec4& value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::vec3& value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::vec2& value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, float value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, int value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, bool value ); 
	static void UpdateUniforms( const std::vector<IUniform*>& rvUniforms, const glm::ivec2& value );
	static void UpdateUniform( IUniform* pIUniform, const glm::mat4& value ); 
	static void UpdateUniform( IUniform* pIUniform, const glm::mat3& value ); 
	static void UpdateUniform( IUniform* pIUniform, const glm::vec4& value ); 
	static void UpdateUniform( IUniform* pIUniform, const glm::vec3& value ); 
	static void UpdateUniform( IUniform* pIUniform, const glm::vec2& value ); 
	static void UpdateUniform( IUniform* pIUniform, float value ); 
	static void UpdateUniform( IUniform* pIUniform, int value ); 
	static void UpdateUniform( IUniform* pIUniform, bool value ); 
	static void UpdateUniform( IUniform* pIUniform, const glm::ivec2& value );

	static bool IsGlobalSemantic( ShaderSemantics::Semantic eSemantic )
	{
		return eSemantic > ShaderSemantics::GLOBAL_UNIFORMS_BEGIN && eSemantic < ShaderSemantics::GLOBAL_UNIFORMS_END;
	}
};

#endif