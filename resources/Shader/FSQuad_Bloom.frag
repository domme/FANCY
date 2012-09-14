#version 330
#define KERNEL_SIZE 9

smooth in vec2 tex;
out vec4 color;

uniform sampler2D tImg;
uniform sampler2D tBloomSource;

uniform bool bUseBloom;

void main( void )                                                                            
{   
	vec4 v4Col = texture2D( tImg, tex );
	vec4 v4Bloom = texture2D( tBloomSource, tex );
	
	
	if( bUseBloom )
		color = v4Col + v4Bloom;

	else
		color = v4Col;
}
                                                                                   
                                                                                         
