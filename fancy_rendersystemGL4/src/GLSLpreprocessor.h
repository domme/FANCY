#ifndef GLSLPREPROCESSOR_H
#define GLSLPREPROCESSOR_H

#include <FancyCorePrerequisites.h>

class DLLEXPORT GLSLpreprocessor
{
	public: 
		static GLSLpreprocessor& getInstance();
		virtual ~GLSLpreprocessor();
	
		void preprocessShader( String& rFinalShaderString, const String& szPath );
	
	private:
		GLSLpreprocessor();
		
	
};



#endif