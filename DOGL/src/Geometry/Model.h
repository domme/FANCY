#ifndef MODEL_H
#define MODEL_H

#include "../Includes.h"
#include "Mesh.h"
#include "BaseGeometryObject.h"

class Model : public BaseGeometryObject
{
	public:
		Model();
		Model( Model& other );
		
		Model( Model&& other );
		Model& operator= ( Model&& other );

		~Model();

		const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return m_vMeshes; }
		void AddMesh( std::unique_ptr<Mesh> pNewMesh );
		
	protected:
		std::vector<std::unique_ptr<Mesh>> m_vMeshes;
};


#endif