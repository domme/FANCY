#version 330

smooth in vec2 tex;
smooth in vec3 v3ViewDir;

out vec4 color;

//G-Buffer Textures
uniform sampler2D colorGloss;
uniform sampler2D specN;
uniform sampler2D normals;
uniform sampler2D depth;

uniform vec3 v3LightPosVS;
uniform vec3 v3LightColor;
uniform float fRstart;
uniform float fRend;

void main( void )
{
	float fGloss = texture2D( colorGloss, tex ).w;
	float fDepth = texture2D( depth, tex ).x;
	vec3 P = v3ViewDir * fDepth;
	vec3 N = normalize( texture2D( normals, tex ).xyz * 2.0 - 1.0 );
	vec4 specColorN = texture2D( specN, tex );

	vec3 L = v3LightPosVS - P;

	float fD = length( L );
	L = normalize( L );
	vec3 H = normalize( normalize( -P ) + L );

	float fFalloff = clamp( ( fRend - fD ) / ( fRend - fRstart ), 0.0, 1.0 );

	float NL = max( 0, dot( N, L ) );
	
	vec3 v3Color = v3LightColor * fFalloff * NL + specColorN.xyz * fFalloff * 2.0 * pow( max( 0, dot( N, H ) ), specColorN.w * 255.0 ); 

	color = vec4( v3Color, 1.0 );
}