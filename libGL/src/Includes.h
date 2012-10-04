#ifndef DOMENGINE_INCLUDES_H
#define DOMENGINE_INCLUDES_H

//STD includes
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <Windows.h>


//OpenGL includes
#include <glew.h>
#include <freeglut.h>



//Math includes
#include <glm/core/setup.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//Vertex types
//#include "Geometry/VertexDeclarations.h"

//Common MACRO defines
#define SAFE_DELETE(p) if(p){ delete p; p = NULL; }
#define SAFE_DELETE_ARR(p) if( p[ 0 ] ) delete[] p;
#define BUFFER_OFFSET(i) ( (char*) NULL + (i) )


template<typename T>
void LOG( T s )
{
	std::ostringstream ss;
	ss << s << '\n';
	OutputDebugStringA( ss.str().c_str() );
}

template<bool T>
void staticAssert_impl()
{
	bool b[ T ];
}

#define STATIC_ASSERT( value ) \
{								\
	staticAssert_impl<value>(); \
}

#define RUN_NOT_FIRST( function ) \
{								\
	static bool bFirst = false; \
								\
	if( bFirst )				\
	{							\
		function;				\
	}							\
								\
	bFirst = true;				\
}

#define RUN_ONLY_ONCE_STATIC( function ) \
{								\
	static bool bInit = false;	\
								\
	if( bInit == true )			\
		return;					\
								\
	function;					\
								\
	bInit = true;				\
}	
													\

#define ARRAY_LENGTH( arr ) sizeof( arr ) / sizeof( arr[ 0 ] )


//DLL-Export MACROS
#ifdef __WINDOWS
	#define DLLEXPORT __declspec(dllexport)
#else
	#define DLLEXPORT 
#endif

#define GLUINT_HANDLE_INVALID 0xFFFFFFFF

typedef std::string String;

typedef glm::uint16				uint16;
typedef glm::uint32				uint32;
typedef glm::uint64				uint64;
typedef glm::uint8				uint8;
typedef glm::uint				uint;
typedef glm::half				float16;


//typedef VertexDeclarations::PosNormTexTanBitan VertexType;



#endif