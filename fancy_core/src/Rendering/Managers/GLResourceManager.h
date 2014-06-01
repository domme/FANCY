#ifndef GLRESOURCE_MANAGER_H
#define GLRESOURCE_MANAGER_H

#include "../../Includes.h"

class DLLEXPORT GLResourceManager
{
public:

	~GLResourceManager();
	void HandleDelete( uint32 uglResource );
	void AddResource( uint32 uglResource );
	bool HasResource( uint32 uglResource );
	int GetResourceUseCount( uint32 uglResource );

protected:
	GLResourceManager();

	virtual void deleteResource( uint32 uglResource );
	virtual String getDebugMgrName() = 0;
	
	typedef std::map<uint32, uint> MapType;
	MapType m_mapResRef;
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLTextureResourceManager : public GLResourceManager
{
public:
	static GLTextureResourceManager& GetInstance() { static GLTextureResourceManager instance; return instance; }

protected:
	GLTextureResourceManager() {};
	virtual void deleteResource( uint32 uglResource );
	virtual String getDebugMgrName() { return "Texture\t"; }
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLVBOResourceManager : public GLResourceManager
{
public:
	static GLVBOResourceManager& GetInstance() { static GLVBOResourceManager instance; return instance; }

protected:
	GLVBOResourceManager() {};
	virtual String getDebugMgrName() { return "VBO\t\t"; }
	virtual void deleteResource( uint32 uglResource );
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLIBOResourceManager : public GLResourceManager
{
public:
	static GLIBOResourceManager& GetInstance() { static GLIBOResourceManager instance; return instance; }

protected:
	GLIBOResourceManager() {};
	virtual String getDebugMgrName() { return "IBO\t\t"; }
	virtual void deleteResource( uint32 uglResource );
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLProgramResourceManager : public GLResourceManager
{
public:
	static GLProgramResourceManager& GetInstance() { static GLProgramResourceManager instance; return instance; }

protected:
	GLProgramResourceManager() {};
	virtual String getDebugMgrName() { return "Shader\t"; }
	virtual void deleteResource( uint32 uglResource );
};

#endif