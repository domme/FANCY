#ifndef NAMEREGISTRY_H
#define NAMEREGISTRY_H

#include "../includes.h"

#include "../Light/Light.h"
#include "../Geometry/Mesh.h"

template<typename object_type>
class DLLEXPORT NameRegistry
{
	friend class ObjectRegistry;
	
	public:
		
		bool						registerObject( const String& szName, object_type* pObject );
		bool						deregisterObject( const String& szName );
		bool						isObjectRegistered( const String& szName );
		object_type*				getObject( const String& szName );
		bool						getObject( const String& szName, object_type** ppObject );  
		void						clearRegistry();
		void						destroyRegistry();
		std::vector<object_type*>	collectAllRegisteredObjects();
		void						collectAllRegisteredObjects( std::vector<object_type*>& rvReturnVec );
		uint						getNumRegisteredObjects();
	
	protected:
		
		NameRegistry();
		virtual ~NameRegistry();
		typedef		std::map<String, object_type*>						ObjectMap;
		
		ObjectMap 				m_mapRegistry;
};




template<typename object_type>
NameRegistry<object_type>::NameRegistry()
{
	
	
}

template<typename object_type>
NameRegistry<object_type>::~NameRegistry()
{
	
}

template<typename object_type>
void NameRegistry<object_type>::destroyRegistry()
{
	if( getNumRegisteredObjects() < 1 )
		return;

	std::map<String, object_type*>::iterator iter;

	for( iter = m_mapRegistry.begin(); iter != m_mapRegistry.end(); ++iter )
	{
		SAFE_DELETE( iter->second );
	}
}

template<typename object_type>
bool NameRegistry<object_type>::registerObject( const String& szName, object_type* pObject )
{
	if( !pObject )
	{
		return false;
	}
	
	if( m_mapRegistry.find( szName ) == m_mapRegistry.end() ) //object not inserted yet!
	{
		m_mapRegistry[ szName ] = pObject;
		return true;
	}
	
	return false;
}

template<typename object_type>
bool NameRegistry<object_type>::deregisterObject( const String& szName )
{
	typename ObjectMap::const_iterator iter;
	
	iter = m_mapRegistry.find( szName );
	if( iter == m_mapRegistry.end() )
	{
		return false;
	}
	
	m_mapRegistry.erase( szName );
	return true;
}

template<typename object_type>
bool NameRegistry<object_type>::isObjectRegistered( const String& szName )
{
	return m_mapRegistry.find( szName ) != m_mapRegistry.end();
}

template<typename object_type>
object_type* NameRegistry<object_type>::getObject( const String& szName )
{
	if( isObjectRegistered( szName ) )
	{
		return m_mapRegistry[ szName ];
	}
	
	return NULL;
}

template<typename object_type>
bool NameRegistry<object_type>::getObject( const String& szName, object_type** ppObject )
{
	( *ppObject ) = getObject( szName );
	
	return ( *ppObject ) != NULL;
}

template<typename object_type>
std::vector<object_type*> NameRegistry<object_type>::collectAllRegisteredObjects()
{
	std::vector<object_type*> returnVec;
	collectAllRegisteredObjects( returnVec );

	return returnVec;
}

template<typename object_type>
void NameRegistry<object_type>::collectAllRegisteredObjects( std::vector<object_type*>& rvReturnVec )
{
	typename ObjectMap::const_iterator iter;

	for( iter = m_mapRegistry.begin(); iter != m_mapRegistry.end(); ++iter )
	{
		rvReturnVec.push_back( iter->second );
	}
}

template<typename object_type>
uint NameRegistry<object_type>::getNumRegisteredObjects()
{
	return m_mapRegistry.size();
}


class ObjectRegistry : public NameRegistry<Mesh>
{
	public:
		static ObjectRegistry& getInstance();
	
	private:
		ObjectRegistry();
		virtual ~ObjectRegistry();
};


class LightRegistry : public NameRegistry<Light>
{
	public:
		static LightRegistry& getInstance();

	private:
		LightRegistry();
		virtual ~LightRegistry();
};





#endif