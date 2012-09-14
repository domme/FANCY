#include "TextureLoader.h"

#include "TextureTGA.h"
#include "../Rendering/Managers/GLResourcePathManager.h"


//#include "../Rendering/Managers/GLResourceManager.h"
#include "PathService.h"
#include "../Services/GLDebug.h"

#include "../Includes.h"


TextureLoader::TextureLoader()
{

}

TextureLoader::~TextureLoader()
{

}

bool TextureLoader::lookupInRegistry( const String& szPath, STextureInfo* pTextureInfo, GLuint& ruTex )
{
	GLTexturePathManager& rTexturePathMgr = GLTexturePathManager::GetInstance();
	TextureInformationRegistry& rTextureInfoRegistry = TextureInformationRegistry::GetInstance();

	if( rTexturePathMgr.HasResource( szPath ) )
	{
		ruTex = rTexturePathMgr.GetResource( szPath );

		const STextureInfo* pTexInfo = rTextureInfoRegistry.GetInfoForTexture( ruTex );

		if( pTextureInfo && pTexInfo )
			*pTextureInfo = *pTexInfo;

		return true;
	}

	return false;
}

GLuint TextureLoader::LoadTexture1D( const String& szPath, bool* pbSuccess /* = NULL */, STextureInfo* pTextureInfo /* = NULL */ )
{
	GLuint uTextureLoc = GLUINT_HANDLE_INVALID;

	if( lookupInRegistry( szPath, pTextureInfo, uTextureLoc ) )
	{
		if( pbSuccess )
			*pbSuccess = true;

		return uTextureLoc;
	}

	if( pbSuccess )
		*pbSuccess = false;

	//////////////////////////////////////////////////////////////////////////
	String szFileType = PathService::getFileType( szPath );
	String szAbsPath =  PathService::convertToAbsPath( szPath );

	uint uWidth;

	if( szFileType == "tga" )
	{
		TextureTGA texture;
		if( !texture.load( szAbsPath ) )
		{
			LOG( std::string( "ERROR: Failed to load Texture: " ) + szPath );
			return 0;
		}

		if( texture.getHeight() > 1 )
			LOG( "ERROR: Texture " + szPath + " Is no 1D-Texture!" );

		uWidth = texture.getWidth();
		
#ifdef _DEBUG
		GLDebug::GL_ErrorCheckStart();
#endif
		glGenTextures( 1, &uTextureLoc );

		glBindTexture( GL_TEXTURE_1D, uTextureLoc );
		glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); 
		glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		//Pass the actual Texture Data
		if( texture.getBitsPerPixel() == 24 )
			glTexImage1D( GL_TEXTURE_1D, 0, GL_SRGB8, texture.getWidth(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture.getImageData() );
		else if( texture.getBitsPerPixel() == 32 )
			glTexImage1D( GL_TEXTURE_1D, 0, GL_SRGB8_ALPHA8, texture.getWidth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getImageData() );
			//glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA8, texture.getWidth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getImageData() );
		else
			LOG( "WARNING: Texture " + szPath + " Has an unsupported pixel format!" );
		glBindTexture( GL_TEXTURE_1D, 0 );
		texture.unload();

#ifdef _DEBUG
		GLDebug::GL_ErrorCheckFinish();
#endif

		if( pbSuccess )
			*pbSuccess = true;
		
	}

	if( uTextureLoc != GLUINT_HANDLE_INVALID )
	{
		STextureInfo sInfo = STextureInfo( uWidth, 1, 1 ); 

		if( pTextureInfo )
			*pTextureInfo = sInfo;
		
		GLTexturePathManager& rTexturePathMgr = GLTexturePathManager::GetInstance();
		TextureInformationRegistry& rTextureInfoRegistry = TextureInformationRegistry::GetInstance();

		rTexturePathMgr.AddResource( szPath, uTextureLoc );
		rTextureInfoRegistry.AddTextureInfo( uTextureLoc, sInfo );
	}

	return uTextureLoc;
}

//TODO: Add import-options like filtering, mip-mapping etc...
GLuint TextureLoader::LoadTexture2D( const String& szPath, bool* pbSuccess /* = NULL */, STextureInfo* pTextureInfo /* = NULL */  )
{
	GLuint uTextureLoc = GLUINT_HANDLE_INVALID;

	if( lookupInRegistry( szPath, pTextureInfo, uTextureLoc ) )
	{
		if( pbSuccess )
			*pbSuccess = true;

		return uTextureLoc;
	}

	if( pbSuccess )
		*pbSuccess = false;

	//Texture not yet in the path manager: load it!
	String szFileType = PathService::getFileType( szPath );
	String szAbsPath =  PathService::convertToAbsPath( szPath );
	
	uint uWidth = 0;
	uint uHeight = 0;

	if( szFileType == "tga" )
	{
		TextureTGA texture;
		if( !texture.load( szAbsPath ) )
		{
			LOG( std::string( "ERROR: Failed to load Texture: " ) + szPath );
			
			if( pbSuccess )
				*pbSuccess = false;

			return GLUINT_HANDLE_INVALID;
		}

		uWidth = texture.getWidth();
		uHeight = texture.getHeight();
	
		glGenTextures( 1, &uTextureLoc );

		glBindTexture( GL_TEXTURE_2D, uTextureLoc );
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, true );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		//Pass the actual Texture Data
		if( texture.getBitsPerPixel() == 24 )
			glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8, texture.getWidth(), texture.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture.getImageData() );
		else if( texture.getBitsPerPixel() == 32 )
			glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texture.getWidth(), texture.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getImageData() );
		else
			LOG( "WARNING: Texture " + szPath + " Has an unsupported pixel format!" );
		glGenerateMipmap( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, 0 );
		texture.unload();
	}
	
	else if( szFileType == "pfm" )
	{
		float* pData = NULL;

		if( !loadPFMfile( szAbsPath, &uWidth, &uHeight, &pData ) )
		{
			LOG( std::string( "ERROR: Failed to load Texture: " ) + szPath );

			if( pbSuccess )
				*pbSuccess = false;

			return GLUINT_HANDLE_INVALID;
		}

		glGenTextures( 1, &uTextureLoc);
		glBindTexture( GL_TEXTURE_2D, uTextureLoc );
		//Pass the actual Texture Data
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, uWidth, uHeight,	0, GL_RGB, GL_FLOAT, pData );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture( GL_TEXTURE_2D, 0 );

		free( pData );
	}

	//The custom filetype ".medical" are Grayscale F32 images
	else if( szFileType == "pbm" )
	{
		
		float* pData = NULL;

		if( !loadPFMfile_Grayscale( szAbsPath, &uWidth, &uHeight, &pData ) )
		{
			LOG( std::string( "ERROR: Failed to load Texture: " ) + szPath );

			if( pbSuccess )
				*pbSuccess = false;

			return GLUINT_HANDLE_INVALID;
		}

		glGenTextures( 1, &uTextureLoc);
		glBindTexture( GL_TEXTURE_2D, uTextureLoc );
		//Pass the actual Texture Data
		glTexImage2D( GL_TEXTURE_2D, 0, GL_R16F, uWidth, uHeight, 0, GL_RED, GL_HALF_FLOAT, pData );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture( GL_TEXTURE_2D, 0 );

		free( pData );
	}
	
	if( uTextureLoc != GLUINT_HANDLE_INVALID )
	{
		if( pbSuccess )
			*pbSuccess = true;

		STextureInfo sInfo = STextureInfo( uWidth, uHeight, 1 ); 

		if( pTextureInfo )
			*pTextureInfo = sInfo;

		GLTexturePathManager& rTexturePathMgr = GLTexturePathManager::GetInstance();
		TextureInformationRegistry& rTextureInfoRegistry = TextureInformationRegistry::GetInstance();

		rTexturePathMgr.AddResource( szPath, uTextureLoc );
		rTextureInfoRegistry.AddTextureInfo( uTextureLoc, sInfo );
	}

	return uTextureLoc;
}

uint numDigits( uint x )
{
	uint iValue = 10;
	uint iDigits = 1;

	while( x >= iValue )
	{
		iValue *= 10;
		iDigits++;
	}

	return iDigits;
}

String uintToPaddedString( uint uValue, uint uMaxDigits )
{
	uint uCurrDigits = numDigits( uValue );

	uint uRequiredPadding = uMaxDigits - uCurrDigits;

	std::stringstream ss;
	for( uint uIPadding = 0; uIPadding < uRequiredPadding; ++uIPadding )
	{
		ss << "0";
	}

	ss << uValue;

	return ss.str();
}



GLuint TextureLoader::LoadTexture3D( const String& szBasePath, uint uStartIndex, uint uEndIndex, const String& szFileType, bool* pbSuccess /* = NULL */, STextureInfo* pTextureInfo /* = NULL */ )
{
	GLuint uTextureLoc = GLUINT_HANDLE_INVALID;

	if( lookupInRegistry( szBasePath, pTextureInfo, uTextureLoc ) )
	{
		if( pbSuccess )
			*pbSuccess = true;

		return uTextureLoc;
	}

	if( pbSuccess )
		*pbSuccess = false;
	
	
	uint uMaxDigits = numDigits( uEndIndex );
	uint uNumTextures = uEndIndex - uStartIndex + 1;

	uint uWidth = 0;
	uint uHeight = 0;
	uint uDepth = uNumTextures;

	String szAbsPath =  PathService::convertToAbsPath( szBasePath );

#ifdef _DEBUG
	LOG( "Loading 3D-Texture " + szBasePath + "..." );
#endif

	if( szFileType == "tga" )
	{
		std::stringstream ss;
		ss << szAbsPath << uintToPaddedString( uStartIndex, uMaxDigits )  << "." << szFileType;

		String szFinalPath = ss.str();

		TextureTGA tex;
		if( ! tex.load( szFinalPath ) )
		{
			LOG( "Texture " + szFinalPath + " failed to load!" );

			if( pbSuccess )
				*pbSuccess = false;

			return 0;
		}

		uHeight = tex.getHeight();
		uWidth = tex.getWidth();
				
		unsigned char* pTexturesData = new unsigned char[ uHeight * uWidth * 3 * uNumTextures ];
		
		for( uint uIdx = 0; uIdx < uHeight * uWidth * 3; ++uIdx )
		{
			pTexturesData[ uIdx ] = tex.getImageData()[ uIdx ];
		}
		tex.unload();


		for( uint iImgIndex = uStartIndex + 1, iTexIdx = 1; iImgIndex <= uEndIndex; ++iImgIndex, ++iTexIdx )
		{
			std::stringstream ss;
			ss << szAbsPath << uintToPaddedString( iImgIndex, uMaxDigits ) << "." << szFileType;

			String szFinalPath = ss.str();

			TextureTGA tex;
			if( ! tex.load( szFinalPath ) )
			{
				LOG( "Texture " + szFinalPath + " failed to load!" );

				if( pbSuccess )
					*pbSuccess = false;

				return 0;
			}


			for( uint uIdx = 0; uIdx < uHeight * uWidth * 3; ++uIdx )
			{
				pTexturesData[ uHeight * uWidth * 3 * ( iTexIdx - 1 ) + uIdx ] = tex.getImageData()[ uIdx ];
			}

			tex.unload();
		}

#ifdef _DEBUG
		LOG( "... Finished loading 3D-Texture " + szBasePath );
#endif
		glGenTextures( 1, &uTextureLoc);
		glBindTexture( GL_TEXTURE_3D, uTextureLoc );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D( GL_TEXTURE_3D, 0, GL_SRGB8, uWidth, uHeight, uNumTextures, 0, GL_RGB, GL_UNSIGNED_BYTE, pTexturesData );
		
		glBindTexture( GL_TEXTURE_3D, 0 );

		delete( pTexturesData );

		if( pbSuccess )
			*pbSuccess = true;
	}

	else if(  szFileType == "pfm" )
	{
		float* pData = NULL;
		
		std::stringstream ss;
		ss << szAbsPath << uintToPaddedString( uStartIndex, uMaxDigits )  << "." << szFileType;

		String szFinalPath = ss.str();
				
		if( !loadPFMfile( szFinalPath, &uWidth, &uHeight, &pData ) )
		{
			LOG( std::string( "ERROR: Failed to load Texture: " ) + szFinalPath );

			if( pbSuccess )
				*pbSuccess = false;

			return 0;
		}

		float* pTexturesData = new float[ uNumTextures * uWidth * uHeight * 3 ];

		for(  uint uIdx = 0; uIdx < uWidth * uHeight * 3; ++uIdx )
		{
			pTexturesData[ uIdx ] = pData[ uIdx ];
		}

		free( pData );

		for( uint iImgIndex = uStartIndex + 1, iTexIdx = 1; iImgIndex <= uEndIndex; ++iImgIndex, ++iTexIdx )
		{
			std::stringstream ss;
			ss << szAbsPath << uintToPaddedString( iImgIndex, uMaxDigits ) << "." << szFileType;

			String szFinalPath = ss.str();

			if( !loadPFMfile( szFinalPath, &uWidth, &uHeight, &pData ) )
			{
				LOG( std::string( "ERROR: Failed to load Texture: " ) + szFinalPath );

				if( pbSuccess )
					*pbSuccess = false;

				return 0;
			}

			for( uint uIdx = 0; uIdx < uWidth * uHeight * 3; ++uIdx )
			{
				pTexturesData[ uWidth * uHeight * ( iImgIndex - 1 ) * 3 + uIdx ] = pData[ uIdx ];
			}

			free( pData );
		}

		
		glGenTextures( 1, &uTextureLoc);
		glBindTexture( GL_TEXTURE_3D, uTextureLoc );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB32F, uWidth, uHeight, uNumTextures, 0, GL_RGB, GL_FLOAT, pTexturesData );

		glBindTexture( GL_TEXTURE_3D, 0 );

		delete( pTexturesData );

		if( pbSuccess )
			*pbSuccess = true;
	}

	else if( szFileType == "volume")
	{
		std::vector<float> vTexturesData;

		String szFilename = szAbsPath + String( ".volume" );
		FILE* pLoadFile = fopen( szFilename.c_str(), "rb" );

		uint uDimensions[ 3 ] = { 0, 0, 0 };

		//Read Header
		fread( uDimensions, sizeof( uint ), 3, pLoadFile );

		uWidth = uDimensions[ 0 ];
		uHeight = uDimensions[ 1 ];
		uNumTextures = uDimensions[ 2 ];
		uDepth = uNumTextures;
		
		vTexturesData.resize( uWidth * uHeight * uNumTextures, 0.0f );
		

		fread( (char*) &vTexturesData[0], sizeof( glm::float16 ), uWidth * uHeight * uNumTextures, pLoadFile );

		fclose( pLoadFile );

		glEnable( GL_TEXTURE_3D );

		glGenTextures( 1, &uTextureLoc);
		glBindTexture( GL_TEXTURE_3D, uTextureLoc );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		//glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
		LOG( "START Generating 3D-Texture" );
		GLDebug::GL_ErrorCheckStart();
		glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE16F_ARB, uWidth, uHeight, uNumTextures, 0, GL_LUMINANCE, GL_HALF_FLOAT, &vTexturesData[0] );
		GLDebug::GL_ErrorCheckFinish();
		LOG( "END Generating 3D-Texture" );

		glBindTexture( GL_TEXTURE_3D, 0 );

		if( pbSuccess )
			*pbSuccess = true;

	}


	else if(  szFileType == "pbm" )
	{
		float* pData = NULL;
	
		std::stringstream ss;
		ss << szAbsPath << uintToPaddedString( uStartIndex, uMaxDigits )  << "." << szFileType;

		String szFinalPath = ss.str();

		if( !loadPFMfile_Grayscale( szFinalPath, &uWidth, &uHeight, &pData ) )
		{
			LOG( std::string( "ERROR: Failed to load Texture: " ) + szFinalPath );

			if( pbSuccess )
				*pbSuccess = false;

			return 0;
		}
	
		std::vector<glm::float16> vTexturesData;
		
		vTexturesData.reserve( uWidth * uHeight * uNumTextures );
		vTexturesData.resize( uWidth * uHeight * uNumTextures, glm::float16( 0.0f ) );

		for(  uint uIdx = 0; uIdx < uWidth * uHeight; ++uIdx )
		{
			vTexturesData[ uIdx ] = glm::float16( pData[ uIdx ] );
		}

		free( pData );

		for( uint iImgIndex = uStartIndex + 1, iTexIdx = 1; iImgIndex <= uEndIndex; ++iImgIndex, ++iTexIdx )
		{
			std::stringstream ss;
			ss << szAbsPath << uintToPaddedString( iImgIndex, uMaxDigits )  << "." << szFileType;

			String szFinalPath = ss.str();

			if( !loadPFMfile_Grayscale( szFinalPath, &uWidth, &uHeight, &pData ) )
			{
				LOG( std::string( "ERROR: Failed to load Texture: " ) + szFinalPath );
				
				if( pbSuccess )
					*pbSuccess = false;

				return 0;
			}

			for( uint uIdx = 0; uIdx < uWidth * uHeight; ++uIdx )
			{
				vTexturesData[ uWidth * uHeight * ( iTexIdx - 1 ) + uIdx ] = glm::float16( pData[ uIdx ] );
			}

			free( pData );
		}

		/*
		String szFilename = szAbsPath + String( ".volume" );
		FILE* pSaveFile = fopen( szFilename.c_str(), "wb" );
		uint uDimensions[ 3 ] = { uWidth, uHeight, uNumTextures };

		fwrite( uDimensions, sizeof( uint ), 3, pSaveFile );
		fwrite( (const char*) &vTexturesData[0], sizeof( glm::float16 ), vTexturesData.size(), pSaveFile );
		fclose( pSaveFile );
		*/

		glGenTextures( 1, &uTextureLoc);
		glBindTexture( GL_TEXTURE_3D, uTextureLoc );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
		LOG( "START Generating 3D-Texture" );
		GLDebug::GL_ErrorCheckStart();
		glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE16F_ARB, uWidth, uHeight, uNumTextures, 0, GL_LUMINANCE, GL_HALF_FLOAT, &vTexturesData[0] );
		GLDebug::GL_ErrorCheckFinish();
		LOG( "END Generating 3D-Texture" );

		glBindTexture( GL_TEXTURE_3D, 0 );

		if( pbSuccess )
			*pbSuccess = true;
		
		/*

		//////////////////////////////////////////////////////////////////////////
		// GPU-Memory Tests
		//////////////////////////////////////////////////////////////////////////
		std::vector<glm::u8vec3> vu8Normals;
		vu8Normals.reserve( uWidth * uHeight * uNumTextures );
		vu8Normals.resize( uWidth * uHeight * uNumTextures, glm::u8vec3( 0, 0, 0) );

		GLuint uTexTempLoc;

		glGenTextures( 1, &uTexTempLoc);
		glBindTexture( GL_TEXTURE_3D, uTexTempLoc );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		LOG( "START Generating Temporal Gradient Texture" );
		GLDebug::GL_ErrorCheckStart();
		glTexImage3DEXT( GL_TEXTURE_3D, 0, GL_RGB8, uWidth, uHeight, uNumTextures, 0, GL_RGB, GL_UNSIGNED_BYTE, &vu8Normals[0] );
		GLDebug::GL_ErrorCheckFinish();
		LOG( "END Generating Temporal Gradient Texture" );
		glBindTexture( GL_TEXTURE_3D, 0 );

		vu8Normals.clear();
		*/
	}	


	if( uTextureLoc != GLUINT_HANDLE_INVALID )
	{
		STextureInfo sInfo = STextureInfo( uWidth, uHeight, uDepth ); 

		if( pTextureInfo )
			*pTextureInfo = sInfo;

		GLTexturePathManager& rTexturePathMgr = GLTexturePathManager::GetInstance();
		TextureInformationRegistry& rTextureInfoRegistry = TextureInformationRegistry::GetInstance();

		rTexturePathMgr.AddResource( szBasePath, uTextureLoc );
		rTextureInfoRegistry.AddTextureInfo( uTextureLoc, sInfo );
	}

	return uTextureLoc;

}


//The Following method has been adopted from the SSDO-example code from "GPU Pro (AK Peters), 2010, edited by Wolfang Engel" and was written by Tobias Ritschel and Thorsten Grosh (http://downloads.akpeters.com/gpupro/)
bool TextureLoader::loadPFMfile( const String& szAbsPath, uint* pWidth, uint* pHeight, float** ppImageData )
{

	// init some variables 
	char imageformat[ 2048 ]; 
	float f[1]; 
	//float* envMapPixels;
	uint envMapWidth = 0;
	uint envMapHeight = 0;

	const char* filename = szAbsPath.c_str();

	// open the file handle  
	FILE* infile = fopen( filename, "rb" ); 

	if ( infile == NULL ) { 
		printf("Error loading %s !\n",szAbsPath.c_str()); 
		return false;
	} 

	// read the header  
	fscanf( infile," %s %d %d ", &imageformat, &envMapWidth, &envMapHeight ); 

	// set member variables 
	// assert( width > 0 && height > 0 ); 
	printf("Image format %s Width %d Height %d\n",imageformat, envMapWidth, envMapHeight ); 

	(*ppImageData)  = (float*) (malloc(envMapWidth * envMapHeight * 3 * sizeof(float))); 

	// go ahead with the data 
	fscanf( infile,"%f", &f[0] ); 
	fgetc( infile ); 

	float red, green, blue; 

	float* p = *ppImageData;

	// read the values and store them 
	for ( unsigned int j = 0; j < envMapHeight ; j++ )  { 
		for ( unsigned int i = 0; i < envMapWidth ; i++ )  { 

			fread( f, 4, 1, infile ); 
			red = f[0]; 

			fread( f, 4, 1, infile ); 
			green = f[0]; 

			fread( f, 4, 1, infile ); 
			blue = f[0]; 

			*p++ = red; 
			*p++ = green; 
			*p++ = blue; 

			float L = (red + green + blue) / 3.0f; 
			//if (L > LMax) 
			//LMax = L; 
		} 
	} 
	printf("Loading Envmap finished\n"); 


	*pWidth = envMapWidth;
	*pHeight = envMapHeight;

	return true;
}


bool TextureLoader::loadPFMfile_Grayscale16( const String& szAbsPath, uint* pWidth, uint* pHeight, glm::float16** ppImageData )
{
	// init some variables 
	char imageformat[ 2048 ]; 
	glm::half f[1]; 
	//float* envMapPixels;
	uint envMapWidth = 0;
	uint envMapHeight = 0;

	const char* filename = szAbsPath.c_str();

	// open the file handle  
	FILE* infile = fopen( filename, "rb" ); 

	if ( infile == NULL ) { 
		printf("Error loading %s !\n",szAbsPath.c_str()); 
		return false;
	} 

	// read the header  
	fscanf( infile," %s %d %d ", &imageformat, &envMapWidth, &envMapHeight ); 

	// set member variables 
	// assert( width > 0 && height > 0 ); 
	printf("Image format %s Width %d Height %d\n",imageformat, envMapWidth, envMapHeight ); 

	(*ppImageData)  = (glm::float16*) (malloc(envMapWidth * envMapHeight * sizeof(glm::float16))); 

	// go ahead with the data 
	fread( f, 2, 1, infile ); 
	//fscanf( infile,"%f", &f[0] ); 
	fgetc( infile ); 

	glm::float16 red, green, blue; 

	glm::float16* p = *ppImageData;

	// read the values and store them 
	for (unsigned  int j = 0; j < envMapHeight ; j++ )  { 
		for ( unsigned int i = 0; i < envMapWidth ; i++ )  { 

			fread( f, 2, 1, infile ); 
			red = static_cast<glm::float16>( f[0] ); 

			*p++ = red; 

			//float L = (red + green + blue) / 3.0; 
			//if (L > LMax) 
			//LMax = L; 
		} 
	} 
	printf("Loading Envmap finished\n"); 

	*pWidth = envMapWidth;
	*pHeight = envMapHeight;

	return true;
}

bool TextureLoader::loadPFMfile_Grayscale( const String& szAbsPath, uint* pWidth, uint* pHeight, float** ppImageData )
{
	// init some variables 
	char imageformat[ 2048 ]; 
	float f[1]; 
	//float* envMapPixels;
	uint envMapWidth = 0;
	uint envMapHeight = 0;

	const char* filename = szAbsPath.c_str();

	// open the file handle  
	FILE* infile = fopen( filename, "rb" ); 

	if ( infile == NULL ) { 
		printf("Error loading %s !\n",szAbsPath.c_str()); 
		return false;
	} 

	// read the header  
	fscanf( infile," %s %d %d ", &imageformat, &envMapWidth, &envMapHeight ); 

	// set member variables 
	// assert( width > 0 && height > 0 ); 
	printf("Image format %s Width %d Height %d\n",imageformat, envMapWidth, envMapHeight ); 

	(*ppImageData)  = (float*) (malloc(envMapWidth * envMapHeight * sizeof(float))); 

	// go ahead with the data 
	fscanf( infile,"%f", &f[0] ); 
	fgetc( infile ); 

	float red; 

	float* p = *ppImageData;

	// read the values and store them 
	for ( unsigned int j = 0; j < envMapHeight ; j++ )  { 
		for ( unsigned int i = 0; i < envMapWidth ; i++ )  { 

			fread( f, 4, 1, infile ); 
			red = static_cast<float>( f[0] ); 

			*p++ = red; 
			
			//float L = (red + green + blue) / 3.0; 
			//if (L > LMax) 
			//LMax = L; 
		} 
	} 
	printf("Loading Envmap finished\n"); 
	
	*pWidth = envMapWidth;
	*pHeight = envMapHeight;

	return true;
}

GLuint TextureLoader::CreateVolumeTransferFunction1D( const std::vector<float>& vIntensities, float fMaxIntensity, const std::vector<glm::vec4>& vColors )
{
	GLuint uTex = 0;



	return uTex;
}


