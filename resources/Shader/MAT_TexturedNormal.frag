#version 330

out vec4 outputColor[4];

smooth in vec3 posV;
smooth in vec3 normV;
smooth in vec3 tanV;
smooth in vec3 bitanV;
smooth in vec2 tex;

uniform sampler2D diffTex;
uniform sampler2D normTex;

uniform float fBumpIntensity;
uniform float fNear;
uniform float fFar;
uniform vec3 v3SpecularColor;
uniform float fSpecExp;
uniform float fGloss;
uniform vec3 v3DiffuseReflectivity;

void main()
{
	//Albedo
	outputColor[0] = vec4( texture2D( diffTex, tex ).xyz, fGloss );

	//Specular
	outputColor[1] = vec4( v3SpecularColor, fSpecExp );

	//Normal
	vec3 v3Nmap = fBumpIntensity * normalize( ( texture2D( normTex, tex ).xyz * 2.0 - 1.0 ) );
	
	vec3 v3FinalNormal = normalize( normV * v3Nmap.z + tanV * v3Nmap.x + bitanV * v3Nmap.y );

	outputColor[2] = vec4( ( v3FinalNormal + 1.0 ) / 2.0, 0.0 );
	
	float fDepth = abs( ( posV.z - fNear )  / ( fFar - fNear ) );
	outputColor[3] = vec4( fDepth );
}