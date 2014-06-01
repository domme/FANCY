#version 330

smooth in vec2 tex;
smooth in vec3 v3ViewDir;

out vec4 color;

//G-Buffer Textures
uniform sampler2D colorGloss;
uniform sampler2D specN;
uniform sampler2D normals;
uniform sampler2D depth;

uniform vec3 v3LightDirVS;
uniform vec3 v3LightColor;

void main( void )
{
	float fGloss = texture2D( colorGloss, tex ).w;
	float fDepth = texture2D( depth, tex ).x;
	vec3 P = v3ViewDir * fDepth;
	vec3 N = normalize( texture2D( normals, tex ).xyz * 2.0 - 1.0 );
	vec4 specColorN = texture2D( specN, tex );
		
	vec3 H = normalize( normalize( -P ) + normalize( -v3LightDirVS ) );
		
	vec3 v3Color = v3LightColor * max( 0, dot( -v3LightDirVS, N ) ) + specColorN.xyz * 2.0 * pow( max( 0, dot( N, H ) ), specColorN.w * 255.0 ); 

	color = vec4( v3Color, 1.0 );
}