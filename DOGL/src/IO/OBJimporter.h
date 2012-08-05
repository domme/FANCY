#pragma once

#include "../includes.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "FileReader.h"
#include "../Geometry/VertexDeclarations.h"
#include "../Rendering/Managers/GLBufferUploader.h"

using namespace std;

class Mesh;
class Model;

class OBJimporter 
{
public:
	static OBJimporter instance;
	static OBJimporter& getInstance();
	
	unique_ptr<Mesh> loadMeshFromFile( const std::string& szFilename, float fUVscaling = 1.0f );
	
	
private:
	
	struct SIndexTupel
	{
		uint uPosIdx;
		uint uTexIdx;
		uint uNormIdx;

		const bool operator<(const SIndexTupel& rhv ) const
		{
			uint ulCombined = uPosIdx + uNormIdx + uTexIdx;
			uint urCombined = rhv.uPosIdx + rhv.uNormIdx + rhv.uTexIdx;

			if( ulCombined != urCombined )
			{
				return ulCombined < urCombined;
			}

			else if( uPosIdx != rhv.uPosIdx )
			{
				return uPosIdx < rhv.uPosIdx;
			}

			else if( uNormIdx != rhv.uNormIdx )
			{
				return uNormIdx < rhv.uNormIdx;
			}

			else
			{
				return uTexIdx < rhv.uTexIdx;
			}
		}

		const bool operator==( const SIndexTupel& rhv ) const
		{
			return uPosIdx == rhv.uPosIdx && uTexIdx == rhv.uTexIdx && uNormIdx == rhv.uNormIdx;
		}
	};

	struct SMtlDesc
	{
		SMtlDesc() : szDiffTexName( "" ), szBumpTexName( "" ), fBumpIntensity( 1 ) {}

		String szDiffTexName;
		String szBumpTexName;
		float fBumpIntensity;
		//....more members to come...
	};

	struct SVertexPos
	{
		glm::vec3 v3Pos;

		const bool operator<(const SVertexPos& rhv ) const
		{
			if( v3Pos.x != rhv.v3Pos.x )
			{
				return v3Pos.x < rhv.v3Pos.x;
			}

			else if( v3Pos.y != rhv.v3Pos.y )
			{
				return v3Pos.y < rhv.v3Pos.y;
			}

			else 
			{
				return v3Pos.z < rhv.v3Pos.z;
			}
		}

		const bool operator==( const SVertexPos& rhv ) const
		{
			return v3Pos.x == rhv.v3Pos.x && v3Pos.y == rhv.v3Pos.y && v3Pos.z == rhv.v3Pos.z;
		}
	};


	//temporary lists for import
	std::vector<glm::vec3>									vertexCoords;
	std::vector<glm::vec2>									textureCoords;
	std::vector<glm::vec3>									normals;

	std::vector<uint>										indices;
	std::vector<uint>										normalIndices;
	std::vector<uint>										textureIndices;


	std::vector<glm::vec3>									m_tan1List;
	std::vector<glm::vec3>									m_tan2List;

	std::vector<Vertex::PosNormTexTan>						m_vVertices;
	std::vector<uint32>										m_vIndices;
	uint16													m_u16PrimitiveType;
	std::map<SIndexTupel, uint>								m_mapTupelNewIndex;
	std::map<String, SMtlDesc>								m_mapMaterialDescs;
	std::map<SVertexPos, uint>								m_mapFaceCounters;
	float													m_fUVScaleFact;

	SMtlDesc*												m_pCurrMtlDesc;

	std::ifstream											m_stream;
	std::ifstream											m_mtlStream;
	char													m_c;
	char													m_mtlC;
	unique_ptr<Mesh>										m_pCurrMesh;
	String													m_szBaseFolder;

	OBJimporter();
	~OBJimporter();

	bool loadMesh( float fUVscaling );
	String getBaseFolder( const String& szFilename );
	void clearTemporalData();

	void parseVertexCoord();
	void parseNormal();
	void parseTextureCoord();
	void parseFaces();
	void paseVertexTypes();
	void parseMeshName();
	void parseMeshUseMaterial();
	void parseMtlName();
	void parseMtlTex();

	void importMtlLib();

	void buildTangentsForLastFace();
	void postProcessVertices();

	GLBufferUploader* m_pBufferUploader;
	
};

