#version 330

smooth in vec2 tex;
smooth in vec3 v3ViewDir;

out vec4 color;

//G-Buffer Textures
uniform sampler2D colorGloss;
uniform sampler2D specN;
uniform sampler2D normals;
uniform sampler2D depth;
//uniform highp sampler2D posTex;

uniform highp samplerCube shadowCubeTex;

uniform vec3 v3LightPosVS;
uniform vec3 v3LightColor;
uniform float fRstart;
uniform float fRend;

uniform float fFar;
uniform float fNear;

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

	vec3 L_WS = ( ( viewI * vec4( -L, 0.0 ) ) ).xyz;
	float fDepthForSM = texture( shadowCubeTex, L_WS ).x * fRend;

	float fDepthForLight = fD; 
	float fShadow = float( fDepthForSM < fDepthForLight - 1.9 ); /* clamp( abs( fDepthForSM - fDepthForLight ), 0.0, 1.0 );  */ 
		
	//float fShadow = texture( shadowCubeTex, vec4( L_WS, fDepthForLight -1000 ) ); //float( abs( fDepthForSM ) < ( fD ) ); 
		
	L = normalize( L );
	vec3 H = normalize( normalize( -P ) + L );
		
	float fFalloff = clamp( ( fRend - fD ) / ( fRend - fRstart ), 0.0, 1.0 );

	float NL = max( 0, dot( N, L ) );
	
	vec3 v3Color = ( 1.0 - fShadow ) * ( ( v3LightColor * fFalloff * NL ) + specColorN.xyz * fFalloff * 2.0 * pow( max( 0, dot( N, H ) ), specColorN.w * 255.0 ) ); 

	
	if( fShadow > 0.0 )
		color = vec4(  1.0, 0.0, 0.0, 1.0 );

	else
		color = vec4( 0.0, 1.0, 0.0, 1.0 ); 
		
	
	//color = vec4( 1.0 - fShadow ); 
	
	//color = vec4( ( fDepthForSM - fDepthForLight ) * ( fDepthForSM - fDepthForLight ) );   
	//color = vec4( textureCube( shadowCubeTex, L_WS ) ).xxxx;

	color = vec4( v3Color, 1.0 ); 
	
		

}