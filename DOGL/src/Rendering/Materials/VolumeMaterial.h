#ifndef VOLUMEMATERIAL_H
#define VOLUMEMATERIAL_H

#include "../../Includes.h"
#include "Material.h"


DLLEXPORT class VolumeMaterial : public Material
{
public:
	VolumeMaterial();
	VolumeMaterial( VolumeMaterial& other );
	~VolumeMaterial();

	virtual Material* Clone() { VolumeMaterial* pClone = CloneVolumeMat(); return pClone; }

	virtual VolumeMaterial* CloneVolumeMat() = 0;

	virtual void UpdateUniform( IUniform* pUniform ) const;

	GLuint GetVolumeTexture() const { return m_uVolumeTexture; }
	const glm::vec3 GetVolumeTextureSize() const { return m_v3VolumeTextureSize; }
	GLuint GetTransferFunctionTexture() const { return m_uTransferFunctionTexture; }
	const glm::vec3 GetTransferFunctionTextureSize() const { return m_v3TransferFunktionSize; }
	GLuint GetRaystartTexture() const { return m_uRaystartTexture; }
	float GetSamplingRate() const { return m_fSamplingRate; }
	float GetWindowValue() const { return m_fWindowValue; }
	GLuint GetShadowTexture() const { return m_uShadowTexture; }
	bool GetUseShadows() const { return m_bUseShadows; }

	void SetVolumeTexture( GLuint uTex ) { m_uVolumeTexture = uTex; }
	void SetVolumeTextureSize( const glm::vec3& v3VolSize ) { m_v3VolumeTextureSize = v3VolSize; }
	void SetTransferFunctionTexture( GLuint uTex ) { m_uTransferFunctionTexture = uTex; }
	void SetTransferFunctionTextureSize( const glm::vec3& v3TrFktSize ) { m_v3TransferFunktionSize = v3TrFktSize; }
	void SetSamplingRate( float fValue ) { m_fSamplingRate = fValue; }
void SetWindowValue( float fValue ) { m_fWindowValue = fValue; } 
	void SetShadowTexture( GLuint uTex ) { m_uShadowTexture = uTex; } 
	void SetUseShadows( bool bShadows ) { m_bUseShadows = bShadows; } 
	static void SetRaystartTexture( GLuint uTex ) { m_uRaystartTexture = uTex; }

	
protected:
	virtual bool validate() const { return m_uVolumeTexture != GLUINT_HANDLE_INVALID && m_uTransferFunctionTexture != GLUINT_HANDLE_INVALID && m_uShadowTexture != GLUINT_HANDLE_INVALID; }

GLuint m_uVolumeTexture;
	glm::vec3 m_v3VolumeTextureSize;

	GLuint m_uTransferFunctionTexture;
	glm::vec3 m_v3TransferFunktionSize;
	
	GLuint m_uShadowTexture;

	static GLuint m_uRaystartTexture;

	float	m_fSamplingRate;
	float	m_fWindowValue;

	bool	m_bUseShadows;
};


#endif