#version 400
#define SHADOW_LAYERS 8

smooth in vec3 pos_interpolated;
smooth in vec3 pos_MS;

out highp vec4 color;

uniform sampler1D texTransferFunction;
uniform highp  sampler2D texRayEndPositions;
uniform highp sampler3D texVolume;
uniform highp sampler3D texDSM;

uniform vec2 screenDimensions;
uniform vec3 clearColor;

uniform bool bNoTransferFunktion;
uniform bool bUseShadows;

uniform mediump vec3 v3CameraPosWS;
uniform mediump vec3 v3LightPosWS;
uniform lowp	vec3 v3LightColorIntensity;
uniform float		 fLightRstart;
uniform float		 fLightRend;
uniform mediump mat4 m4ModelWorldI;

uniform mediump mat4 m4LightProj;
uniform mediump mat4 m4ModelWorldLightView;

uniform mediump float fSamplingRate;
uniform mediump float fWindowValue;
uniform mediump vec3 v3VolumeSize;


//globals
highp float fDSM_layerStep = 1.0 / float( SHADOW_LAYERS );
highp float fDSM_DepthEpsilon = 0.01;
lowp int iVoxelSkipping = 5;

mediump vec3 calcGradient_forward( in float fCentralValue, in vec3 v3CenterCoord, in vec3 v3Steps )
{
	mediump vec3 v3ForwardValues;

	v3ForwardValues.x = texture3D( texVolume, v3CenterCoord + vec3( v3Steps.x, 0.0, 0.0 ) ).x;
	v3ForwardValues.y = texture3D( texVolume, v3CenterCoord + vec3( 0.0, v3Steps.y, 0.0 ) ).x;
	v3ForwardValues.z = texture3D( texVolume, v3CenterCoord + vec3( 0.0, 0.0, v3Steps.z ) ).x;

	return ( v3ForwardValues - vec3( fCentralValue ) ) / v3Steps; 
}

mediump vec3 calcGradient_central( in vec3 v3CenterCoord, in vec3 v3Steps, in float fVolumeValue, out float fFilteredVolumeValue )
{
	mediump vec3 v3ForwardValues;
	mediump vec3 v3BackwardValues;
		
	v3ForwardValues.x =  texture3D( texVolume, v3CenterCoord + vec3( v3Steps.x, 0.0, 0.0 ) ).x	;
	v3ForwardValues.y =  texture3D( texVolume, v3CenterCoord + vec3( 0.0, v3Steps.y, 0.0 ) ).x	;
	v3ForwardValues.z =  texture3D( texVolume, v3CenterCoord + vec3( 0.0, 0.0, v3Steps.z ) ).x	;
	v3BackwardValues.x = texture3D( texVolume, v3CenterCoord + vec3( -v3Steps.x, 0.0, 0.0 ) ).x	;
	v3BackwardValues.y = texture3D( texVolume, v3CenterCoord + vec3( 0.0, -v3Steps.y, 0.0 ) ).x	;
	v3BackwardValues.z = texture3D( texVolume, v3CenterCoord + vec3( 0.0, 0.0, -v3Steps.z ) ).x	;

	fVolumeValue += v3ForwardValues.x;
	fVolumeValue += v3ForwardValues.y;
	fVolumeValue += v3ForwardValues.z;
	fVolumeValue += v3BackwardValues.x;
	fVolumeValue += v3BackwardValues.y;
	fVolumeValue += v3BackwardValues.z; 

	fFilteredVolumeValue = fVolumeValue / 7.0;

	return ( v3BackwardValues - v3ForwardValues ) / ( 1.0 + 2.0 * v3Steps ); 
}


void main( void )
{
	color = vec4( 0.0 );

	vec2 uv = gl_FragCoord.xy / screenDimensions;
	highp vec3 endPosition = texture2D( texRayEndPositions, uv ).xyz;
	highp vec3 ray =  endPosition - pos_interpolated;

	highp float fLength = length( ray );
	
	ray = normalize( ray );

	mediump vec3 v3CameraPosMS = ( m4ModelWorldI * vec4( v3CameraPosWS, 1.0 ) ).xyz;
	mediump vec3 v3LightPosMS = ( m4ModelWorldI * vec4( v3LightPosWS, 1.0 ) ).xyz;
	
	int iNumSamples = int( fSamplingRate * v3VolumeSize.z ); 
	highp float fAdvance = 1.0 / float( iNumSamples );
	highp vec3 v3VoxelSteps = 1.0 / v3VolumeSize;

	lowp vec4 v4Color = vec4( 0.0 );

	int i = 0;
	bool bEmpty = false;
	bool bBacktrack = false;
	for( ; i < iNumSamples; i++ )
	{
		float len = i * fAdvance;

		if( len > fLength )
			break;

		highp vec3 v3CurrSamplePos = pos_interpolated + ray * len;
		highp float fVolumeValue = texture3D( texVolume, v3CurrSamplePos ).x;
						
		if( bBacktrack == false && fVolumeValue < fWindowValue )
		{
			i+=iVoxelSkipping; //leave out the next voxels
			bEmpty = true;
			
			continue;
		}

		else if( bEmpty == true )
		{
			bEmpty = false;
			bBacktrack = true;
			i -=iVoxelSkipping;

			continue;
		}

		else if( bBacktrack == true )
		{
			bBacktrack = false;
		}

		//Calculate the final Volume value by filtering the value with its neighbours. Also compute gradient in the same time
		highp float fFilteredVolumeValue = 0.0;
		mediump vec3 v3Gradient = calcGradient_central( v3CurrSamplePos, v3VoxelSteps, fVolumeValue, fVolumeValue ); 
		
		highp vec3 v3CurrPosMS = v3CurrSamplePos * 2.0 - 1.0;
				
		highp vec4 v4VolumeColorA = vec4( 0.0 );
		
				
		v4VolumeColorA = vec4( texture1D( texTransferFunction, fVolumeValue ).xyz, fVolumeValue );

		
		highp vec2 v2DSMkey = vec2( 1.0 );
		mediump float fShadow = 1.0;

		highp vec3 v3CurrPosLS = ( m4ModelWorldLightView * vec4( v3CurrPosMS, 1.0 ) ).xyz;
		highp float fCamDepth = abs( v3CurrPosLS.z );

		if( bUseShadows )
		{
			highp vec4 v4ShadowMapSample = ( m4LightProj * vec4( v3CurrPosLS, 1.0 ) );
			highp vec2 v2ShadowMapSample = ( ( v4ShadowMapSample.xy / v4ShadowMapSample.w ) + 1.0 ) / 2.0;
			
			highp vec4 v4DSMsample = texture3D( texDSM, vec3( v2ShadowMapSample, 0.0 ) );

			int iSamples = 30;
			float fShadowStep = 1.0 / float( iSamples );
			
			float fMinDiff = 1.0;
			
			for( int iShadow = 0; iShadow < iSamples; iShadow++ )
			{
				
				highp vec4 v4DSMsample = texture3D( texDSM, vec3( v2ShadowMapSample, float( iShadow ) * fShadowStep ) );
				
				v2DSMkey.x = v4DSMsample.x;
				v2DSMkey.y = v4DSMsample.y;

				float fDiff = abs( fCamDepth - v2DSMkey.x );

				if( fDiff < fMinDiff - 0.019 )
				{
					fMinDiff = fDiff;
					fShadow = 1.0 - v2DSMkey.y;
				}

				v2DSMkey.x = v4DSMsample.z;
				v2DSMkey.y = v4DSMsample.w;

				fDiff = abs( fCamDepth - v2DSMkey.x );

				if( fDiff < fMinDiff - 0.019 )
				{
					fMinDiff = fDiff;
					fShadow = 1.0 - v2DSMkey.y;
				}

			}
			
			if( fMinDiff > 0.1 )
			{	
				fShadow = 0.05;
			}
		}
				

		mediump vec3 v3LightDirMS = ( v3LightPosMS - v3CurrPosMS );
		mediump float fLightDistance = length( v3LightDirMS );
		v3LightDirMS /= fLightDistance;

		mediump float fAtt = ( fLightRend - fLightDistance ) / ( fLightRend - fLightRstart );
				
		mediump float fGradientMagnitude = length( v3Gradient );
		v3Gradient /= fGradientMagnitude;

		
		mediump float fDiffPhong = 0.0;
				
		if( fGradientMagnitude > 0.001 )
		{
			mediump vec3 v3V = normalize( v3CameraPosMS - v3CurrPosMS );
			mediump vec3 v3H = normalize( v3LightDirMS + v3V );
			mediump float fDiffuseAngle = dot( v3LightDirMS, v3Gradient );
		
			fDiffPhong = ( max( 0.0001, fDiffuseAngle ) + float( fDiffuseAngle > 0.0001 ) * max( 0.0, pow( dot( v3H, v3Gradient ), 20.0 ) ) );
		}
		
		mediump float fLightAttenuation = fAtt * fShadow * fDiffPhong;
					
		//Opacity-correction
		v4VolumeColorA.a = clamp( 1.0 - pow( ( 1.0 - v4VolumeColorA.a ), ( 1.0 / fSamplingRate ) ), 0.0, 1.0 );	
		v4VolumeColorA.xyz *= v4VolumeColorA.a * v3LightColorIntensity * fLightAttenuation;
		
		v4Color += ( ( 1.0 - v4Color.a ) * v4VolumeColorA );

		if( v4Color.a > 0.99 )
			break;
	}  


	color = (0.99 - v4Color.a) * vec4( clearColor, 0.0 ) + v4Color;

}
