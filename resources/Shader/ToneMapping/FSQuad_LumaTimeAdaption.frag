#version 330

smooth in vec2 tex;

//The input texture
uniform sampler2D currentFrameLuma;
uniform sampler2D lastFrameLuma;

uniform float fAdaptionRate;

out vec4 color;

void main( void )
{
	vec4 vLumaCurr = texture2D( currentFrameLuma, tex );
	vec4 vLumaLast = texture2D( lastFrameLuma, tex );

	color = ( 1.0 - fAdaptionRate ) * vLumaLast + fAdaptionRate * vLumaCurr;
}