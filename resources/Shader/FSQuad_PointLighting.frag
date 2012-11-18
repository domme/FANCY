#version 330

smooth in vec2 tex;
smooth in vec3 v3ViewDir;

out vec4 color;

//G-Buffer Textures
uniform sampler2D colorGloss;
uniform sampler2D specN;
uniform sampler2D normals;
uniform sampler2D depth;

uniform samplerCube shadowCubeTex;

uniform vec3 v3LightPosVS;
uniform vec3 v3LightColor;
uniform float fRstart;
uniform float fRend;
uniform mat4 lightView;
uniform mat4 lightProj;
uniform mat4 viewI;



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

	vec3 pLightView = ( lightView * ( viewI * vec4( P, 1.0 ) ) ).xyz;
	float fDepthForSM = textureCube( shadowCubeTex, pLightView ).x;
	float fDepthForLight = length( pLightView ); 

	float fVisible = float( abs( fDepthForLight ) - 0.0005 < abs( fDepthForSM ) ); 

	float fFalloff = clamp( ( fRend - fD ) / ( fRend - fRstart ), 0.0, 1.0 );

	float NL = max( 0, dot( N, L ) );
	
	vec3 v3Color = fVisible * ( v3LightColor * fFalloff * NL  + specColorN.xyz * fFalloff * 2.0 * pow( max( 0, dot( N, H ) ), specColorN.w * 255.0 ) ); 

	//color = vec4( v3Color, 1.0 );
	color = textureCube( shadowCubeTex, pLightView );
}