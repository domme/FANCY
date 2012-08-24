
#ifndef MATERIAL_H
#define MATERIAL_H

#include "../../Includes.h"
#include "../Shader.h"
#include "../GLTexture.h"
#include "../IUniform.h"

class Mesh;

namespace ESamplingDirection
{
	enum eSamplingDirection
	{
		DIRECTION_HORIZONTAL,
		DIRECTION_VERTICAL
	};
}


struct SMaterialDescription
{
	uint8 uNumTexturesDiffuse;
	uint8 uNumTexturesNormal;
	uint8 uNumTexturesGloss;
	uint8 uNumTexturesSpecular;
};

class Material
{
public:
	Material();
	Material( Material& other );
	~Material();
	
	virtual bool Init() = 0;
	virtual GLuint GetTextureAtIndex( uint uIdx ) const { return 0; }
	virtual const glm::vec3& GetTextureSizeAtIndex( uint uIdx ) const = 0;
	virtual Material* Clone() = 0;

	virtual void UpdateUniform( IUniform* pUniform ) const;

	Shader* GetShader() { return m_pShader; }
	const Shader* GetShader() const { return m_pShader; }

	const glm::vec3& GetAmbientReflectivity() const { return m_v3AmbientReflectivity; }
	void SetAmbientReflectivity( const glm::vec3& ambient ) { m_v3AmbientReflectivity = ambient; }

	const glm::vec3& GetDiffuseReflectivity() const { return m_v3DiffuseReflectivity; }
	void SetDiffuseReflectivity( const glm::vec3 diffuse ) { m_v3DiffuseReflectivity = diffuse; }

	const glm::vec3& GetColor() const { return m_v3Color; }
	void SetColor( const glm::vec3& color ) { m_v3Color = color; }

	float GetOpacity() const { return m_fOpacity; }
	void SetOpacity( float fOpacity ) { m_fOpacity = fOpacity; }

	const glm::vec3&	GetSpecularColor() const { return m_v3SpecularColor; }
	void				SetSpecularColor( const glm::vec3& v3Spec ) { m_v3SpecularColor = v3Spec; }

	float				GetSpecularExponent() const { return m_fSpecularExponent; }
	void				SetSpecularExponent( float fN ) { m_fSpecularExponent = fN; }

	float				GetGlossiness() const { return m_fGlossiness; }
	void				SetGlossiness( float fG ) { m_fGlossiness = fG; }

	void				ValidateMaterial() const;
	
protected:
	Shader* m_pShader;
	glm::vec3 m_v3AmbientReflectivity;
	glm::vec3 m_v3Color;
	glm::vec3 m_v3SpecularColor;
	glm::vec3 m_v3DiffuseReflectivity;
	float m_fSpecularExponent;
	float m_fGlossiness;
	float m_fOpacity;

	virtual bool validate() const = 0;

	static void assignAttributeSemantic( Shader* pShader, const String& szName, ShaderSemantics::Semantic eSemantic );
	static void assignUniformSemantic( Shader* pShader, const String& szName, ShaderSemantics::Semantic eSemantic );
	static void assignUniformSemantic( Shader* pShader, const String& szName, ShaderSemantics::Semantic eSemantic, int index );
	
};


#endif