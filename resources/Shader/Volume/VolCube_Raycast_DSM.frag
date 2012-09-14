#version 400

#define NUM_LAYERS 8
#define F_ERROR 0.06

smooth in vec3 pos_interpolated;

out highp vec4 color[ NUM_LAYERS ];

uniform highp sampler2D texRayEndPositions;
uniform highp sampler3D texVolume;
uniform highp sampler1D texTransferFunction;

uniform vec2 screenDimensions;

uniform mediump mat4 m4ModelWorldView;
uniform mediump float fSamplingRate;
uniform mediump float fWindowValue;
uniform mediump vec3 v3VolumeSize;


void storeNewValue( float fOpacity, float fDepth, int numSavedElement )
{
	int iLayerNum = numSavedElement / 2;
	int iElementPlace = int( mod( numSavedElement, 2 ) );
		
	if( iElementPlace == 0 )
	{
		color[ iLayerNum ].x = fDepth;
		color[ iLayerNum ].y = fOpacity;
	}

	else 
	{
		color[ iLayerNum ].z = fDepth;
		color[ iLayerNum ].w = fOpacity;
	}
}

void main( void )
{		
	vec2 uv = gl_FragCoord.xy /screenDimensions;
	highp vec3 endPosition = texture2D( texRayEndPositions, uv ).xyz;
	highp vec3 ray =  endPosition - pos_interpolated;
	highp float fLength = length( ray );
		
	ray = normalize( ray );
	
	lowp float fRealSamplingRate = 2.0;

	int iNumSamples = int( fRealSamplingRate * v3VolumeSize.z ); 
	highp float fAdvance = 1.0 / float( iNumSamples );

	lowp float fOpacity = 0.0;
	lowp float fLastSaved = 0.0;
		

	int iNumSavedElements = 0;
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
			i+=3; //leave out the next voxels
			bEmpty = true;
			
			continue;
		}

		else if( bEmpty == true )
		{
			bEmpty = false;
			bBacktrack = true;
			i -=3;

			continue;
		}

		else if( bBacktrack == true )
		{
			bBacktrack = false;
		}
		
		
		float fCurrError = abs( fOpacity - fLastSaved );		
	
		if( iNumSavedElements == 0 || fCurrError > F_ERROR  )
		{
			mediump vec3 v3CurrPosMS = v3CurrSamplePos * 2.0 -1.0;
			highp float fLightDepth = abs( ( m4ModelWorldView * vec4( v3CurrPosMS, 1.0 ) ).z );
			storeNewValue( fOpacity, fLightDepth, iNumSavedElements );
			iNumSavedElements++;
						
			fLastSaved = fOpacity;
		}

		if( fOpacity > 0.99 || iNumSavedElements > NUM_LAYERS * 2 - 1 )
			break;

		//Sampling-Rate Correction
		fVolumeValue = max( 0.0, ( 1.0 -  pow( ( 1.0 - fVolumeValue ), ( 1.0 / fRealSamplingRate ) ) ) );	
		fOpacity += ( ( 1.0 - fOpacity ) * fVolumeValue );
	}

	for( int i = iNumSavedElements; i < NUM_LAYERS * 2 - 1; ++i )
		storeNewValue( 1.0, 1.0, i );
	
}
