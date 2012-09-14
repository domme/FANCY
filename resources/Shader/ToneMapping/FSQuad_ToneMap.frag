#version 330

smooth in vec2 tex;

//The input texture
uniform sampler2D inputTex;
uniform sampler2D avgLuminanceTex;
uniform bool	  bToneMappingEnabled;

uniform float exposure = 0.5;

out vec4 color;

void main( void )
{
	color = texture2D( inputTex, tex );
	

	if( bToneMappingEnabled )
	{
		float currL = dot( color.rgb, vec3( 0.299, 0.587, 0.114 ) );
		float avgL = texture2DLod( avgLuminanceTex, vec2( 0.5, 0.5 ), 7 ).r;
		//float targetL = exposure * ( exposure/avgL + 1.0 ) / (exposure + 1);
		//color.rgb = sqrt( color.rgb * targetL );
	
		float targetL =  ( ( exposure * 5.0 ) / avgL ) * currL;
		color.rgb = color.rgb * targetL;
		color.rgb = color.rgb / ( 1.0 + color.rgb ); 
	}
	
	color.rgb = sqrt( color.rgb ); //gamma-correction
	color.a = sqrt( dot( color.rgb, vec3( 0.299, 0.587, 0.114 ) ) ); //compute luma and store it in a-channel 
}