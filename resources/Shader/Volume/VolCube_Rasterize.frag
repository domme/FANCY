#version 330

smooth in vec3 pos_interpolated;

out vec4 color;

void main( void )
{
	//Render out the interpolated position to use it as texture coordinate in subsequent rendering passes.
	color = vec4( pos_interpolated, 1.0 );
}