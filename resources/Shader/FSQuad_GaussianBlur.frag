#version 330

smooth in vec2 tex;
out vec4 color;

uniform int kernelSize;

uniform sampler2D tImg;
uniform sampler1D tGauss;
uniform sampler1D tOffsets;

uniform vec2 v2ImageSize;  
uniform vec2 v2SamplingDir;

float fGaussOffsetStep = 1.0 / float(kernelSize);

//float weights[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );
//float offsets[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );

float offset( int x )
{
	return texture1D( tOffsets, float(x) * fGaussOffsetStep ).r; 
}
                                                                                         
float gauss( int x )                                                                   
{    
	return texture1D( tGauss, float(x) * fGaussOffsetStep ).r;
}
                                                                                         
vec4 blurGauss()                                                                         
{                                                                                        
   float fStep;                                                                           
   vec2 centerOffset;                                                                    
                                                                                         
   if( v2SamplingDir.x > 0.0 )                                                           
   {                                                                                     
   		fStep = 1.0 / v2ImageSize.x;                                                      
   		centerOffset = vec2( 0.5, 0.0 ) * fStep;                                          
   }                                                                                     
                                                                                         
   else                                                                                  
   {                                                                                     
   		fStep = 1.0 / v2ImageSize.y;                                                      
   		centerOffset = vec2( 0.0, 0.5 ) * fStep;                                          
   }                                                                                     
                                                    
   
   float fWeights = gauss( 0 );
   vec4 v4Color = texture2D( tImg, tex + centerOffset ) * fWeights;
   
   //float k = 1.0;
   
   for( int i = 1; i < kernelSize; ++i )                                  
   {        
		float fWeight = gauss( i );
		fWeights += 2.0 * fWeight;
   		vec2 v2SamplingPos = ( tex + centerOffset ) + v2SamplingDir * offset( i ) * fStep;
		v4Color += texture2D( tImg, v2SamplingPos ) * fWeight;
		
		v2SamplingPos = ( tex + centerOffset ) - v2SamplingDir * offset( i ) * fStep;                                                                                                                           
		v4Color += texture2D( tImg, v2SamplingPos ) * fWeight;
		
		//k += 2.0;
   }                                                                                     
   	                                                                                     
   return v4Color / fWeights;                                                            
}

void main( void )                                                                            
{   
	fGaussOffsetStep = 1.0 / float(kernelSize);
	color = vec4( blurGauss().xyz, texture2D( tImg, tex ).a );
   
	//color = vec4( 1.0, 0.0, 0.0, 1.0 );
   //color = texture2D( tImg, tex );
}
                                                                                   
                                                                                         
