#include "TextureLoader.h"
#include "../Rendering/Managers/GLResourcePathManager.h"


//#include "../Rendering/Managers/GLResourceManager.h"
#include "PathService.h"
#include "../Services/GLDebug.h"

#include "../Includes.h"

#include "lodepng.h"

TextureLoader::TextureLoader()
{

}

TextureLoader::~TextureLoader()
{

}

bool TextureLoader::lookupInRegistry( const String& szPath, STextureInfo* pTextureInfo, uint32& ruTex )
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

uint32 TextureLoader::LoadTexture1D( const String& szPath, bool* pbSuccess /* = NULL */, STextureInfo* pTextureInfo /* = NULL */ )
{
  uint32 uTextureLoc = GLUINT_HANDLE_INVALID;

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

  if( szFileType == "png" )
  {
    std::vector<unsigned char> imageData;
    std::vector<unsigned char> buffer;

    lodepng::load_file( buffer, szAbsPath );

    uint width, height;
    lodepng::State pngState;
    pngState.decoder.color_convert = false;
    //	uint err = lodepng::decode( imageData, width, height, pngState, reinterpret_cast<const unsigned char*>( szAbsPath.c_str() ), static_cast<size_t>( szAbsPath.size() ) );
    //uint err = lodepng::decode( imageData, width, height, szAbsPath );
    //uint err = lodepng::decode( imageData, width, height, pngState, vPath );
    uint err = lodepng::decode( imageData, width, height, pngState, buffer );

    if( err != 0 )
    {
      std::stringstream errMsgStream;
      errMsgStream << lodepng_error_text( err );
      LOG( std::string( "ERROR: Failed to load Texture: " ) + szPath  + ": " + errMsgStream.str() );

      if( pbSuccess )
        *pbSuccess = false;

      return GLUINT_HANDLE_INVALID;
    }

    LodePNGColorMode& color = pngState.info_raw;

    uWidth = width;
    
    glGenTextures( 1, &uTextureLoc );

    glBindTexture( GL_TEXTURE_1D, uTextureLoc );
    glTexParameteri( GL_TEXTURE_1D, GL_GENERATE_MIPMAP, true );
    glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
    glTexParameterf( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    //Pass the actual Texture Data
    if( color.colortype == LCT_RGB )
      glTexImage1D( GL_TEXTURE_1D, 0, GL_SRGB8, width, 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*> ( &imageData[0] ) );
    else if( color.colortype == LCT_RGBA )
      glTexImage1D( GL_TEXTURE_1D, 0, GL_SRGB8_ALPHA8, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*> ( &imageData[0] ) );
     
    glGenerateMipmap( GL_TEXTURE_1D );
    glBindTexture( GL_TEXTURE_1D, 0 );
    
  }

  if( uTextureLoc != GLUINT_HANDLE_INVALID )
  {
    if( pbSuccess )
      *pbSuccess = true;

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
uint32 TextureLoader::LoadTexture2D( const String& szPath, bool* pbSuccess /* = NULL */, STextureInfo* pTextureInfo /* = NULL */  )
{
  uint32 uTextureLoc = GLUINT_HANDLE_INVALID;

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

  if( szFileType == "png" )
  {
    std::vector<unsigned char> imageData;
    std::vector<unsigned char> buffer;

    lodepng::load_file( buffer, szAbsPath );

    uint width, height;
    lodepng::State pngState;
    pngState.decoder.color_convert = false;
  //	uint err = lodepng::decode( imageData, width, height, pngState, reinterpret_cast<const unsigned char*>( szAbsPath.c_str() ), static_cast<size_t>( szAbsPath.size() ) );
    //uint err = lodepng::decode( imageData, width, height, szAbsPath );
    //uint err = lodepng::decode( imageData, width, height, pngState, vPath );
    uint err = lodepng::decode( imageData, width, height, pngState, buffer );

    if( err != 0 )
    {
      std::stringstream errMsgStream;
      errMsgStream << lodepng_error_text( err );
      LOG( std::string( "ERROR: Failed to load Texture: " ) + szPath  + ": " + errMsgStream.str() );

      if( pbSuccess )
        *pbSuccess = false;

      return GLUINT_HANDLE_INVALID;
    }

    LodePNGColorMode& color = pngState.info_raw;
    
    uWidth = width;
    uHeight = height;

    glGenTextures( 1, &uTextureLoc );

    glBindTexture( GL_TEXTURE_2D, uTextureLoc );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, true );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    //Pass the actual Texture Data
    if( color.colortype == LCT_RGB )
      glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*> ( &imageData[0] ) );
    else if( color.colortype == LCT_RGBA )
      glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*> ( &imageData[0] ) );

    glGenerateMipmap( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 0 );
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