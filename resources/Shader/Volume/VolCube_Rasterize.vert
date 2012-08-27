#version 330

layout (location = 0) in vec3 pos;

smooth out vec3 pos_interpolated;
smooth out vec3 pos_MS;

uniform mat4 MWVP;

void main(void)
{
	//Cube-Vertices are [-1..1]. Interpolated Position needs to be [0..1] for use as texture coordinates.
	pos_MS = pos;
	pos_interpolated = ( pos + 1.0 ) / 2.0;
	gl_Position = MWVP * vec4( pos, 1.0 );
}

