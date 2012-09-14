#version 330

//////////////////////////////////////////////////////////////////////////
//Dom-Engine Integration defines
//////////////////////////////////////////////////////////////////////////
#define FXAA_PC 1
#define FXAA_GLSL_130 1
#define FXAA_QUALITY__PRESET 39
#define FXAA_GREEN_AS_LUMA 0
//////////////////////////////////////////////////////////////////////////

//Note: self-made "include" only works from the "shader" directory
#include "FXAA\\Fxaa3_11.h"

smooth in vec2 tex;
smooth in vec3 v3ViewDir;

//uniform vec2 viewportSize;
uniform vec2 pixelStep;

//The input texture
uniform sampler2D inputTexture;

out vec4 color;

//Dummies used fot the not-needed parameters of FXAA
FxaaFloat4 _dummyVec4 = vec4( 0.0 );
FxaaFloat _dummyFloat = 0.0;
		
FxaaFloat fxaaQualitySubpix = 0.50;
FxaaFloat fxaaQualityEdgeThreshold = 0.063;
FxaaFloat fxaaQualityEdgeThresholdMin = 0.0312;

void main( void )
{
	//center of the pixel
	//FxaaFloat2 pos = tex; // + vec2( -1.0, -1.0 ) * ( pixelStep / 2.0 );
			
	color = FxaaPixelShader(	tex,							//pos
								_dummyVec4,						//fxaaXonsolePosPos
								inputTexture,					//tex
								inputTexture,					//fxaaConsole360TexExpBiasNegOne
								inputTexture,					//fxaaConsole360TexExpBiasNegTwo
								pixelStep,						//fxaaQualityRcpFrame
								_dummyVec4,						//fxaaConsoleRcpFrameOpt
								_dummyVec4,						//fxaaConsoleRcpFrameOpt2
								_dummyVec4,						//fxaaConsole360RcpFrameOpt2
								fxaaQualitySubpix,				//fxaaQualitySubpix
								fxaaQualityEdgeThreshold,		//fxaaQualityEdgeThreshold
								fxaaQualityEdgeThresholdMin,	//fxaaQualityEdgeThresholdMin
								_dummyFloat,					//fxaaConsoleEdgeSharpness
								_dummyFloat,					//fxaaConsoleEdgheThreshold
								_dummyFloat,					//fxaaConsoleEdgheThresholdMin
								_dummyVec4						//fxaaConsole360ConstDir
							);

}