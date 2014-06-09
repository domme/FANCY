#ifndef GLRESOURCE_MANAGER_H
#define GLRESOURCE_MANAGER_H

#include "FancyCorePrerequisites.h"

class DLLEXPORT GPUResourceManager
{
public:

	~GPUResourceManager();
	void HandleDelete( uint32 uglResource );
	void AddResource( uint32 uglResource );
	bool HasResource( uint32 uglResource );
	int GetResourceUseCount( uint32 uglResource );

protected:
	GPUResourceManager();

	virtual void deleteResource( uint32 uglResource );
	virtual String getDebugMgrName() = 0;
	
	typedef std::map<uint32, uint> MapType;
	MapType m_mapResRef;
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT TextureResourceManager : public GPUResourceManager
{
public:
	static TextureResourceManager& GetInstance() { static TextureResourceManager instance; return instance; }

protected:
	TextureResourceManager() {};
	virtual void deleteResource( uint32 uglResource );
	virtual String getDebugMgrName() { return "Texture\t"; }
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT VBOResourceManager : public GPUResourceManager
{
public:
	static VBOResourceManager& GetInstance() { static VBOResourceManager instance; return instance; }

protected:
	VBOResourceManager() {};
	virtual String getDebugMgrName() { return "VBO\t\t"; }
	virtual void deleteResource( uint32 uglResource );
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT IBOResourceManager : public GPUResourceManager
{
public:
	static IBOResourceManager& GetInstance() { static IBOResourceManager instance; return instance; }

protected:
	IBOResourceManager() {};
	virtual String getDebugMgrName() { return "IBO\t\t"; }
	virtual void deleteResource( uint32 uglResource );
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GPUProgramResourceManager : public GPUResourceManager
{
public:
	static GPUProgramResourceManager& GetInstance() { static GPUProgramResourceManager instance; return instance; }

protected:
	GPUProgramResourceManager() {};
	virtual String getDebugMgrName() { return "Shader\t"; }
	virtual void deleteResource( uint32 uglResource );
};

#endif