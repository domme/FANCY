#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 MWVP;
uniform mat4 MWV;
uniform mat4 MWVIT;

smooth out vec3 posV;
smooth out vec3 normV;
smooth out vec2 tex;

void main()
{
	gl_Position = MWVP * vec4( position, 1.0 );

	posV = ( MWV * vec4( position, 1.0 ) ).xyz;
	normV = normalize( ( MWVIT * vec4( normal, 0.0 ) ).xyz );
	tex = uv;
}