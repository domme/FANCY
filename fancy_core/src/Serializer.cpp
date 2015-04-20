#include "Serializer.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace Internal
  {
    

    struct TextureHeader
    {
      ShortStringDesc myPath;
      uint32 myWidth;
      uint32 myHeight;
      uint32 myDepth;
      uint32 myFormat;
      uint32 myAccessFlags;
      uint32 myPixelDataSizeBytes;
      uint32 myNumMipmapLevels;
    };


    struct TransformDesc
    {
      glm::quat m_localRotation;
      glm::vec3 m_localPosition;
      glm::vec3 m_localScale;
    };

    struct SceneNodeDesc
    {
      ShortStringDesc myName;

    };

  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  bool SerializerBinary::serialize(Scene::SceneNode** someSceneNode, const String& someSerializePath)
  {
    if (myMode == ESerializationMode::STORE)
    {
      return store(someSceneNode, someSerializePath);
    }
    else
    {
      return false;
     // return load(_aTexture, _aSerializePath);
    }
  }
//---------------------------------------------------------------------------//
  bool SerializerBinary::serialize(Rendering::Texture** _aTexture, const void* _aData, uint32 _aDataSize, const String& _aSerializePath)
  {
    if (myMode == ESerializationMode::STORE)
    {
      return store(_aTexture, _aData, _aDataSize, _aSerializePath);
    }
    else
    {
      return load(_aTexture, _aSerializePath);
    }
  }
//---------------------------------------------------------------------------//
  bool SerializerBinary::store(Scene::SceneNode** someSceneNode, const String& someSerializePath)
  {

  }
//---------------------------------------------------------------------------//
  bool SerializerBinary::store(Rendering::Texture** _aTexture, const void* _aData, uint32 _aDataSize, const String& _aSerializePath)
  {
    std::ofstream aStream(_aSerializePath, std::ios::binary | std::ios::out);
    ASSERT(aStream.good());

    Rendering::Texture* aTexture = *_aTexture;

    const Rendering::TextureDesc& texParams = aTexture->getParameters();

    Internal::TextureHeader header;
    header.myPath = aTexture->getPath();
    header.myWidth = texParams.u16Width;
    header.myHeight = texParams.u16Height;
    header.myDepth = texParams.u16Depth;
    header.myAccessFlags = texParams.uAccessFlags;
    header.myPixelDataSizeBytes = _aDataSize;
    header.myFormat = static_cast<uint32>(texParams.eFormat);
    header.myNumMipmapLevels = texParams.u8NumMipLevels;

    aStream.write(reinterpret_cast<const char*>(&header), sizeof(Internal::TextureHeader));
    aStream.write(static_cast<const char*>(_aData), _aDataSize);

    return aStream.good();
  }
//---------------------------------------------------------------------------//
  bool SerializerBinary::load(Rendering::Texture** _aTexture, const String& _aSerializePath)
  {
    std::ifstream aStream(_aSerializePath, std::ios::binary | std::ios::in);

    if (!aStream.good())
    {
      return false;
    }

    // read the size of the header
    Internal::TextureHeader header;
    aStream.read((char*)&header, sizeof(Internal::TextureHeader));

    std::vector<char> aPixelData;
    aPixelData.resize(header.myPixelDataSizeBytes, 0);
    aStream.read((char*)&aPixelData[0], header.myPixelDataSizeBytes);

    Rendering::TextureDesc texParams;
    texParams.path = header.myPath.toString();
    texParams.u16Width = header.myWidth;
    texParams.u16Height = header.myHeight;
    texParams.u16Depth = header.myDepth;
    texParams.eFormat = static_cast<Rendering::DataFormat>(header.myFormat);
    texParams.u8NumMipLevels = header.myNumMipmapLevels;
    texParams.uAccessFlags = header.myAccessFlags;
    texParams.uPixelDataSizeBytes = header.myPixelDataSizeBytes;
    texParams.pPixelData = &aPixelData[0];

    if ((*_aTexture) == nullptr)
    {
      (*_aTexture) = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);
    }

    (*_aTexture)->create(texParams);

    return true;
  }
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::IO 