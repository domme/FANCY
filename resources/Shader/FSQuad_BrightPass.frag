#version 330
#define KERNEL_SIZE 9

smooth in vec2 tex;
out vec4 color;

uniform sampler2D tImg;

void main( void )                                                                            
{   
	vec4 v4Col = texture2D( tImg, tex );
    color = max( vec4( 0.0 ), v4Col - vec4( 0.2, 0.2, 0.2, 0.0 ) );
}
                                                                                   
                                                                                         
