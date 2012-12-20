#ifndef GLRESOURCEPATH_MANAGER_H
#define GLRESOURCEPATH_MANAGER_H

#include "../../Includes.h"

class DLLEXPORT GLResourcePathManager
{
public:
	~GLResourcePathManager();

	bool HasResource( const String& szPath );
	GLuint GetResource( const String& szPath );
	virtual void AddResource( const String& szPath, GLuint uResource );
	virtual void HandleResourceDeletion( GLuint uResource );

protected:
	GLResourcePathManager();

	typedef std::map<String, GLuint> MapType;
	MapType m_mapPathRes;
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLTexturePathManager : public GLResourcePathManager
{
public:
	static GLTexturePathManager& GetInstance() { static GLTexturePathManager instance; return instance; }

	virtual void HandleResourceDeletion( GLuint uResource );
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLShaderPathManager : public GLResourcePathManager
{
public:
	static GLShaderPathManager& GetInstance() { static GLShaderPathManager instance; return instance; }

};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLVBOpathManager : public GLResourcePathManager
{
public:
	static GLVBOpathManager& GetInstance() { static GLVBOpathManager instance; return instance; }

};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT GLIBOpathManager : public GLResourcePathManager
{
public:
	static GLIBOpathManager& GetInstance() { static GLIBOpathManager instance; return instance; }

};
#endif