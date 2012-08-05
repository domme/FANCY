#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../includes.h"

	class DLLEXPORT TransformSQT
	{
		public:
			TransformSQT();
			TransformSQT( const glm::mat4& rMat );
			~TransformSQT();
		
			
			TransformSQT concatenate( const TransformSQT& rOterTransform ) const;
			const glm::mat4 getAsMat4() const;
			void setData( const TransformSQT& rOtherTransform );
			void appendToMatrix( glm::mat4& rMat );

			glm::vec3 m_v3Translation;
			glm::vec3 m_v3Scale;
			glm::quat m_quatRotation;

	};	


#endif