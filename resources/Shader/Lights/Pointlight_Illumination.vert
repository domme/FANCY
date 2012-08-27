#version 330

layout (location = 0) in vec3 position;

uniform float fRend;
uniform vec3 v3PointlightPosVS;

uniform mat4 matProj;

void main()
{
	gl_Position = matProj * vec4( position * fRend * 1.2 + v3PointlightPosVS, 1.0 );
}