#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 MWVP;
uniform mat4 MWV;
uniform mat4 MWVIT;

smooth out vec3 posV;
smooth out vec3 normV;
smooth out vec3 tanV;
smooth out vec3 bitanV;
smooth out vec2 tex;

void main()
{
	gl_Position = MWVP * vec4( position, 1.0 );

	posV = ( MWV * vec4( position, 1.0 ) ).xyz;
	normV = normalize( ( MWVIT * vec4( normal, 0.0 ) ).xyz );
	tanV = normalize( ( MWVIT * vec4( tangent, 0.0 ) ).xyz );
	bitanV = normalize( cross( normV, tanV ) );

	tex = uv;
}