#version 330
#extension  GL_EXT_gpu_shader4 : enable

uniform sampler2D img;

smooth in vec2 uv_interpolated;

out vec4 color;

void main( void )
{
	color = texture2D( img, uv_interpolated ); 
}