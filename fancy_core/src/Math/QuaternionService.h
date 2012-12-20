#ifndef QUATERNION_SERVICE
#define QUATERNION_SERVICE

#include "../includes.h"

class DLLEXPORT QuaternionService
{
	public:
		static glm::quat CreateRotationQuaternion( const float fAngle, const glm::vec3& rAxis )
		{
			glm::vec3 v3NormAxis = glm::normalize( rAxis ); 
			const float fHalfAngle = fAngle / 2.0f;

			glm::quat quatRotation;
			quatRotation.x = v3NormAxis.x * sin( fHalfAngle );
			quatRotation.y = v3NormAxis.y * sin( fHalfAngle );
			quatRotation.z = v3NormAxis.z * sin( fHalfAngle );
			quatRotation.w = cos( fHalfAngle );
			quatRotation = glm::normalize( quatRotation );

			return quatRotation;
		}
};

#endif