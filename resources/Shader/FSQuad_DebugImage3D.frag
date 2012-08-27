#version 330
#extension  GL_EXT_gpu_shader4 : enable

uniform sampler3D img;
uniform float time;

smooth in vec2 uv_interpolated;

out vec4 color;

void main( void )
{
	color = texture3D( img, vec3( uv_interpolated.x, uv_interpolated.y, time ) ); 

	//color = vec4( texture3D( img, vec3( uv_interpolated.x, uv_interpolated.y, 0.5 ) ).y ); 
}