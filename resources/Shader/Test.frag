#version 330

out vec4 outputColor[4];

smooth in vec3 posV;
smooth in vec3 normV;
smooth in vec3 tanV;
smooth in vec3 bitanV;
smooth in vec2 tex;

uniform mat4 WV;

const vec4 lightDirW = vec4( -1.0, -1.0, 1.0, 0.0 );

void main()
{
	vec3 lightDir = normalize( ( WV * lightDirW ).xyz );
	outputColor[0] = vec4( 0.0, 1.0, 0.0, 1.0 );// * max( 0.0, dot( lightDir, normV ) );
	outputColor[1] = vec4( 1.0, 1.0, 1.0, 1.0 );
	outputColor[2] = vec4( normalize( normV ), 1.0 );

	float fDepth = abs( posV.z / 1000.0 );
	outputColor[3] = vec4( fDepth );
}