#version 330

out vec4 outputColor[5];

smooth in vec3 posV;
smooth in vec3 normV;
smooth in vec3 tanV;
smooth in vec3 bitanV;
smooth in vec2 tex;

uniform sampler2D diffTex;
uniform sampler2D normTex;
uniform sampler2D specTex;
uniform sampler2D glossTex;

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
	outputColor[0] = vec4( texture2D( diffTex, tex ).xyz, clamp( fGloss * texture2D( glossTex, tex ).x, 0.0, 1.0 ) );

	//Specular
	outputColor[1] = vec4( clamp( v3SpecularColor * texture2D( specTex, tex ).xyz, 0.0, 1.0 ), fSpecExp );

	//Normal
	vec3 v3Nmap = ( texture2D( normTex, tex ).xyz );
	
	vec3 v3FinalNormal = normalize( normV + tanV * v3Nmap.x + bitanV * v3Nmap.y );

	outputColor[2] = vec4( ( v3FinalNormal + 1.0 ) / 2.0, 0.0 );
	
	//depth
	float fDepth = abs( posV.z / fFar ); //abs( ( posV.z - fNear )  / ( fFar - fNear ) );
	outputColor[3] = vec4( fDepth );
	
	//Pos
	outputColor[4] = vec4( posV, 1.0 );
}