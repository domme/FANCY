#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

smooth out vec2 uv_interpolated;

void main(void)
{
	gl_Position = vec4( pos, 1.0 );
	uv_interpolated = uv;
}

