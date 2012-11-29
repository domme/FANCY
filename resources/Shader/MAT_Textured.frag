#version 330

out vec4 outputColor[4];

smooth in vec3 posV;
smooth in vec3 normV;
smooth in vec2 tex;

uniform sampler2D diffTex;
uniform float fNear;
uniform float fFar;
uniform vec3 v3SpecularColor;
uniform float fSpecExp;
uniform float fGloss;
uniform vec3 v3DiffuseReflectivity;

void main()
{
	//AlbedoGloss
	outputColor[0] = vec4( texture2D( diffTex, tex ).xyz, fGloss );

	//SpecN
	outputColor[1] = vec4( v3SpecularColor, fSpecExp );

	//Norm
	outputColor[2] = vec4( ( normalize( normV ) + 1.0 ) / 2.0, 1.0 );

	//Depth
	float fDepth = abs( posV.z / fFar ); //abs( ( posV.z - fNear )  / ( fFar - fNear ) );
	outputColor[3] = vec4( fDepth );

	//Pos
	//outputColor[4] = vec4( posV, 1.0 );
}