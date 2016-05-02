#if !defined (INCLUDE_CONSTANTBUFFER_INCLUDE_)
#define INCLUDE_CONSTANTBUFFER_INCLUDE_
//---------------------------------------------------------------------------//
  struct PER_FRAME
  {
    float4 c_TimeParamters;
  };
//---------------------------------------------------------------------------//
  struct PER_VIEWPORT
  {
    float4 c_RenderTargetSize;
  };
//---------------------------------------------------------------------------//
  struct PER_CAMERA
  {
    float4x4 c_ViewMatrix;
    float4x4 c_ViewInverseMatrix;
    float4x4 c_ProjectionMatrix;
    float4x4 c_ProjectionInverseMatrix;
    float4x4 c_ViewProjectionMatrix;
    float4x4 c_ViewProjectionInverseMatrix;
    float4 c_NearFarParameters;
    float4 c_CameraPosWS;
  };
//---------------------------------------------------------------------------//
  struct PER_LIGHT
  {
    // xyz: ColorIntensity
    // w: LightType (Dir, Point, Spot, Area)
    float4 c_LightParameters;
    float4 c_PointSpotParameters;
    float3 c_LightPosWS;
    float3 c_LightPosVS;
    float3 c_LightDirWS;
    float3 c_LightDirVS;
  };
//---------------------------------------------------------------------------//  
  struct PER_MATERIAL
  {
    float4 c_MatDiffIntensity;
    float4 c_MatSpecIntensity;
  };
//---------------------------------------------------------------------------//  
  struct PER_OBJECT
  {
      float4x4 c_WorldMatrix;
      float4x4 c_WorldInverseMatrix;
      float4x4 c_WorldViewMatrix;
      float4x4 c_WorldViewInverseMatrix;
      float4x4 c_WorldViewProjectionMatrix;
      float4x4 c_WorldViewProjectionInverseMatrix;
  };
//---------------------------------------------------------------------------//  
#endif // INCLUDE_CONSTANTBUFFER_INCLUDE_