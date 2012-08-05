#ifndef GLRESOURCE_MANAGER_H
#define GLRESOURCE_MANAGER_H

#include "../../Includes.h"

class GLResourceManager
{
public:

	~GLResourceManager();
	void HandleDelete( GLuint uglResource );
	void AddResource( GLuint uglResource );
	bool HasResource( GLuint uglResource );
	int GetResourceUseCount( GLuint uglResource );

protected:
	GLResourceManager();

	virtual void deleteResource( GLuint uglResource );
	virtual String getDebugMgrName() = 0;
	
	typedef std::map<GLuint, uint> MapType;
	MapType m_mapResRef;
};

//////////////////////////////////////////////////////////////////////////
class GLTextureResourceManager : public GLResourceManager
{
public:
	static GLTextureResourceManager& GetInstance() { static GLTextureResourceManager instance; return instance; }

protected:
	GLTextureResourceManager() {};
	virtual void deleteResource( GLuint uglResource );
	virtual String getDebugMgrName() { return "Texture\t"; }
};


//////////////////////////////////////////////////////////////////////////
class GLVBOResourceManager : public GLResourceManager
{
public:
	static GLVBOResourceManager& GetInstance() { static GLVBOResourceManager instance; return instance; }

protected:
	GLVBOResourceManager() {};
	virtual String getDebugMgrName() { return "VBO\t\t"; }
	virtual void deleteResource( GLuint uglResource );
};

//////////////////////////////////////////////////////////////////////////
class GLIBOResourceManager : public GLResourceManager
{
public:
	static GLIBOResourceManager& GetInstance() { static GLIBOResourceManager instance; return instance; }

protected:
	GLIBOResourceManager() {};
	virtual String getDebugMgrName() { return "IBO\t\t"; }
	virtual void deleteResource( GLuint uglResource );
};


//////////////////////////////////////////////////////////////////////////
class GLProgramResourceManager : public GLResourceManager
{
public:
	static GLProgramResourceManager& GetInstance() { static GLProgramResourceManager instance; return instance; }

protected:
	GLProgramResourceManager() {};
	virtual String getDebugMgrName() { return "Shader\t"; }
	virtual void deleteResource( GLuint uglResource );
};

#endif