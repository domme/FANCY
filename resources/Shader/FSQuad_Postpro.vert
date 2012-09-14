#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

smooth out vec2 tex;
smooth out vec3 v3ViewDir;

uniform float fRatio;
uniform float fYfov;
uniform float fFar;
uniform int   iScreenHeight;

void main(void)
{
	tex = uv;

	float h2 = fFar * tan( fYfov / 2.0 );
	float w2 = fRatio * h2;

	v3ViewDir = vec3( pos.xy * vec2( w2, h2 ), -fFar );

	gl_Position = vec4( pos, 1.0 );
}