#ifndef UNIFORM_H
#define UNIFORM_H

#include "../Includes.h"
#include "IUniform.h"
#include "Shader.h"
#include "ShaderSemantics.h"
#include "UniformRegistry.h"
#include "UniformUtil.h"

#define EPSILON 0.001f

template<typename ValueT>
class DLLEXPORT Uniform : public IUniform
{
	friend class Shader;
	friend class GLRenderer;

public:
	Uniform() : m_iglHandle( -1 ), m_szName( "Uninizialized" ), m_pObserver( NULL ), m_eSemantic( ShaderSemantics::UNDEFINED ), m_iN( -1 ) {}
	Uniform( const Uniform<ValueT>* other );
	
	virtual ~Uniform() {}
	

	const ValueT&					GetValue() const { return m_clValue; }
	
	virtual int						GetIndex() const { return m_iN; }
	virtual void					SetIndex( int iN ) { m_iN = iN; }
	virtual int32					GetGLhandle() const { return m_iglHandle; }
	virtual ShaderSemantics::Semantic GetSemantic() const { return m_eSemantic; }
	
	//Has to be inline for some reason...
	virtual IUniform* SetSemantic( ShaderSemantics::Semantic eSemantic )
	{
		UniformRegistry& rRegistry = UniformRegistry::GetInstance();

		if( m_eSemantic != ShaderSemantics::UNDEFINED )
			rRegistry.RemoveUniform( this );


		m_eSemantic = eSemantic;
        
		if( UniformUtil::IsGlobalSemantic( m_eSemantic ) )
			rRegistry.RegisterUniformForSemantic( this, m_eSemantic );

		return this;
	}

	virtual const String& GetName() const { return m_szName; }
	
	void SetValue( const ValueT& value ) 
	{ 
		m_clValue = value;
		notifyValueChange();

		/*if( valueIsDifferent( value ) )
		{
			m_clValue = value;

			notifyValueChange();
		}*/
	}

	virtual void SetObserver( IUniformObserver* pObserver ) { if( pObserver ) m_pObserver = pObserver; }
	
protected:
	ValueT						m_clValue;
	int32						m_iglHandle;
	String						m_szName;
	bool						m_bRecalculate;
	ShaderSemantics::Semantic	m_eSemantic;
	int							m_iN;

	
	IUniformObserver* m_pObserver;

	void notifyValueChange()
	{
		if( m_pObserver )
			m_pObserver->UniformChanged( this );
	}

	void cleanup();
	virtual bool valueIsDifferent( const ValueT& value ) const = 0;
	static bool differentFloat( float f1, float f2 ) { return glm::abs( f1 - f2 ) > EPSILON; }
};

template<typename ValueT>
void Uniform<ValueT>::cleanup()
{
	if( UniformUtil::IsGlobalSemantic( m_eSemantic ) )
	{
		LOG( "Cleaning up uniform " + m_szName );

		UniformRegistry& rRegistry = UniformRegistry::GetInstance();

		rRegistry.RemoveUniform( this );
	}
}


//////////////////////////////////////////////////////////////////////////
// List specialized Uniform-Derivations here
//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformVec2 : public Uniform<glm::vec2> 
{
protected:
	virtual bool valueIsDifferent( const glm::vec2& value ) const
	{
		return  differentFloat( value.x, m_clValue.x ) ||
				differentFloat( value.y, m_clValue.y );
	}

public:
	UniformVec2() : Uniform<glm::vec2>::Uniform() {}
	UniformVec2( const UniformVec2* other ) : Uniform<glm::vec2>::Uniform( other ) {}
	virtual ~UniformVec2() { Uniform<glm::vec2>::cleanup(); }

	virtual IUniform* Clone() const { return new UniformVec2( *this );  }
	
	virtual void Upload() const
	{
		glUniform2fv( m_iglHandle, 1, glm::value_ptr( m_clValue ) );		
	}	
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformVec3 : public Uniform<glm::vec3>
{
protected:
	virtual bool valueIsDifferent( const glm::vec3& value ) const
	{
		return	differentFloat( value.x, m_clValue.x ) ||
				differentFloat( value.y, m_clValue.y ) ||
				differentFloat( value.z, m_clValue.z );
	}

public:
	UniformVec3( const UniformVec3* other ) : Uniform<glm::vec3>::Uniform( other ) {}
	UniformVec3() : Uniform<glm::vec3>( ) {}

	virtual ~UniformVec3() { Uniform<glm::vec3>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformVec3( *this ); }

	virtual void Upload() const
	{
		glUniform3fv( m_iglHandle, 1, glm::value_ptr( m_clValue ) );		
	}	
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformVec4 : public Uniform<glm::vec4>
{
protected:
	virtual bool valueIsDifferent( const glm::vec4& value ) const
	{
		return	differentFloat( value.x, m_clValue.x ) ||
			differentFloat( value.y, m_clValue.y ) ||
			differentFloat( value.z, m_clValue.z ) ||
			differentFloat( value.w, m_clValue.w );
	}

public:
	UniformVec4( const UniformVec4* other ) : Uniform<glm::vec4>::Uniform( other ) {}
	UniformVec4() : Uniform<glm::vec4>::Uniform( ) {}

	virtual ~UniformVec4() { Uniform<glm::vec4>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformVec4( *this ); }

	virtual void Upload() const
	{
		glUniform4fv( m_iglHandle, 1, glm::value_ptr( m_clValue ) );		
	}	
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformFloat : public Uniform<float>
{
protected:
	virtual bool valueIsDifferent( const float& value ) const
	{
		return differentFloat( value, m_clValue );
	}

public:
	UniformFloat() : Uniform<float>::Uniform() {}
	UniformFloat( const UniformFloat* other ) : Uniform<float>::Uniform( other ) {}

	virtual ~UniformFloat() { Uniform<float>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformFloat( *this ); }

	virtual void Upload() const
	{
		glUniform1f( m_iglHandle, m_clValue );
	}
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformInt : public Uniform<int>
{
protected:
	virtual bool valueIsDifferent( const int& value ) const
	{
		return value != m_clValue;
	}

public:
	UniformInt( const UniformInt* other ) : Uniform<int>::Uniform( other ) {}
	UniformInt() : Uniform<int>::Uniform() {}

	virtual ~UniformInt() { Uniform<int>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformInt( *this ); } 

	virtual void Upload() const
	{
		glUniform1i(  m_iglHandle, m_clValue );
	}
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformMat2 : public Uniform<glm::mat2>
{
protected:
	virtual bool valueIsDifferent( const glm::mat2& value ) const
	{
		return	differentFloat( value[0][0], m_clValue[0][0] ) ||
				differentFloat( value[0][1], m_clValue[0][1] ) ||
				differentFloat( value[1][0], m_clValue[1][0] ) ||
				differentFloat( value[1][1], m_clValue[1][1] );
	}

public:
	UniformMat2( const UniformMat2* other ) : Uniform<glm::mat2>::Uniform( other ) {}
	UniformMat2() : Uniform<glm::mat2>::Uniform() {}

	virtual ~UniformMat2(){ Uniform<glm::mat2>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformMat2( *this ); }

	virtual void Upload() const
	{
		glUniformMatrix2fv( m_iglHandle, 1, GL_FALSE, glm::value_ptr( m_clValue ) );
	}
};

//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformMat3 : public Uniform<glm::mat3>
{
protected:
	virtual bool valueIsDifferent( const glm::mat3& value ) const
	{
		return 	differentFloat( value[0][0], m_clValue[0][0] ) ||
				differentFloat( value[0][1], m_clValue[0][1] ) ||
				differentFloat( value[0][2], m_clValue[0][2] ) ||

				differentFloat( value[1][0], m_clValue[1][0] ) ||
				differentFloat( value[1][1], m_clValue[1][1] ) ||
				differentFloat( value[1][2], m_clValue[1][2] ) ||

				differentFloat( value[2][0], m_clValue[2][0] ) ||
				differentFloat( value[2][1], m_clValue[2][1] ) ||
				differentFloat( value[2][2], m_clValue[2][2] );
	}

public:
	UniformMat3( const UniformMat3* other ) : Uniform<glm::mat3>::Uniform( other ) {}
	UniformMat3() : Uniform<glm::mat3>::Uniform() {}

	virtual ~UniformMat3(){ Uniform<glm::mat3>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformMat3( *this ); }

	virtual void Upload() const
	{
		glUniformMatrix3fv( m_iglHandle, 1, GL_FALSE, glm::value_ptr( m_clValue ) );
	}
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformMat4 : public Uniform<glm::mat4>
{
protected:
	virtual bool valueIsDifferent( const glm::mat4& value ) const
	{
		return  differentFloat( value[0][0], m_clValue[0][0] ) ||
				differentFloat( value[0][1], m_clValue[0][1] ) ||
				differentFloat( value[0][2], m_clValue[0][2] ) ||
				differentFloat( value[0][3], m_clValue[0][3] ) ||

				differentFloat( value[1][0], m_clValue[1][0] ) ||
				differentFloat( value[1][1], m_clValue[1][1] ) ||
				differentFloat( value[1][2], m_clValue[1][2] ) ||
				differentFloat( value[1][3], m_clValue[1][3] ) ||

				differentFloat( value[2][0], m_clValue[2][0] ) ||
				differentFloat( value[2][1], m_clValue[2][1] ) ||
				differentFloat( value[2][2], m_clValue[2][2] ) ||
				differentFloat( value[2][3], m_clValue[2][3] ) ||

				differentFloat( value[3][0], m_clValue[3][0] ) ||
				differentFloat( value[3][1], m_clValue[3][1] ) ||
				differentFloat( value[3][2], m_clValue[3][2] ) ||
				differentFloat( value[3][3], m_clValue[3][3] );
	}

public:
	UniformMat4( const UniformMat4* other ) : Uniform<glm::mat4>::Uniform( other ) {}
	UniformMat4() : Uniform<glm::mat4>::Uniform() {}

	virtual ~UniformMat4() { Uniform<glm::mat4>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformMat4( *this ); }

	virtual void Upload() const
	{
		glUniformMatrix4fv( m_iglHandle, 1, GL_FALSE, glm::value_ptr( m_clValue ) );
	}
};


///////////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* x-Component: Texture Unit index
   y-Component: Texture Handle											*/
/************************************************************************/
class DLLEXPORT UniformTexture1D : public Uniform<glm::ivec2>
{
protected:
	virtual bool valueIsDifferent( const glm::ivec2& value ) const
	{
		return	value.x != m_clValue.x ||
				value.y != m_clValue.y;
	}

public:
	UniformTexture1D( const UniformTexture1D* other ) : Uniform<glm::ivec2>::Uniform( other ) {}
	UniformTexture1D() : Uniform<glm::ivec2>::Uniform() {}

	virtual ~UniformTexture1D() { Uniform<glm::ivec2>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformTexture1D( *this ); }

	virtual void Upload() const
	{
		glUniform1i( m_iglHandle, m_clValue.x );
		glActiveTexture( GL_TEXTURE0 + m_clValue.x );
		glBindTexture( GL_TEXTURE_1D, m_clValue.y );
	}
};

///////////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* x-Component: Texture Unit index
   y-Component: Texture Handle											*/
/************************************************************************/
class DLLEXPORT UniformTexture2D : public Uniform<glm::ivec2>
{
protected:
	virtual bool valueIsDifferent( const glm::ivec2& value ) const
	{
		return	value.x != m_clValue.x ||
				value.y != m_clValue.y;
	}

public:
	UniformTexture2D( const UniformTexture2D* other ) : Uniform<glm::ivec2>::Uniform( other ) {}
	UniformTexture2D() : Uniform<glm::ivec2>::Uniform() {}

	virtual ~UniformTexture2D() { Uniform<glm::ivec2>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformTexture2D( *this ); }

	virtual void Upload() const
	{
		glUniform1i( m_iglHandle, m_clValue.x );
		glActiveTexture( GL_TEXTURE0 + m_clValue.x );
		glBindTexture( GL_TEXTURE_2D, m_clValue.y );
	}
};

///////////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* x-Component: Texture Unit index
   y-Component: Texture Handle											*/
/************************************************************************/
class DLLEXPORT UniformTexture3D : public Uniform<glm::ivec2>
{
protected:
	virtual bool valueIsDifferent( const glm::ivec2& value ) const
	{
		return	value.x != m_clValue.x ||
				value.y != m_clValue.y;
	}

public:
	UniformTexture3D( const UniformTexture3D* other ) : Uniform<glm::ivec2>::Uniform( other ) {}
	UniformTexture3D() : Uniform<glm::ivec2>::Uniform() {}

	virtual ~UniformTexture3D() { Uniform<glm::ivec2>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformTexture3D( *this ); }

	virtual void Upload() const
	{
		glUniform1i( m_iglHandle, m_clValue.x );
		glActiveTexture( GL_TEXTURE0 + m_clValue.x );
		glBindTexture( GL_TEXTURE_3D, m_clValue.y );
	}
};

///////////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* x-Component: Texture Unit index
   y-Component: Texture Handle											*/
/************************************************************************/
class DLLEXPORT UniformTextureCube : public Uniform<glm::ivec2>
{
protected:
	virtual bool valueIsDifferent( const glm::ivec2& value ) const
	{
		return	value.x != m_clValue.x ||
			value.y != m_clValue.y;
	}

public:
	UniformTextureCube( const UniformTextureCube* other ) : Uniform<glm::ivec2>::Uniform( other ) {}
	UniformTextureCube() : Uniform<glm::ivec2>::Uniform() {}

	virtual ~UniformTextureCube() { Uniform<glm::ivec2>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformTextureCube( *this ); }

	virtual void Upload() const
	{
		glUniform1i( m_iglHandle, m_clValue.x );
		glActiveTexture( GL_TEXTURE0 + m_clValue.x );
		glBindTexture( GL_TEXTURE_CUBE_MAP, m_clValue.y );
	}
};


//////////////////////////////////////////////////////////////////////////
class DLLEXPORT UniformBool : public Uniform<bool>
{
protected:
	virtual bool valueIsDifferent( const bool& value ) const
	{
		return value != m_clValue;
	}

public:
	UniformBool( const UniformBool* other ) : Uniform<bool>::Uniform( other ) {}
	UniformBool() : Uniform<bool>::Uniform() {}

	virtual ~UniformBool() { Uniform<bool>::cleanup(); }
	virtual IUniform* Clone() const { return new UniformBool( *this ); }

	virtual void Upload() const
	{
		glUniform1i( m_iglHandle, m_clValue );
	}
};


#endif