#version 330

smooth in vec2 tex;
smooth in vec3 v3ViewDir;

out vec4 color;

//G-Buffer Textures
uniform sampler2D colorGloss;
uniform sampler2D localIllum;


void main( void )
{
	color = vec4( texture2D( colorGloss, tex ).xyz * texture2D( localIllum, tex ).xyz, 1.0 );
}