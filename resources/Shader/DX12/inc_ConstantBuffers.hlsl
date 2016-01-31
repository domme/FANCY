#if !defined (INCLUDE_CONSTANTBUFFER_INCLUDE_)
#define INCLUDE_CONSTANTBUFFER_INCLUDE_
//---------------------------------------------------------------------------//
  struct PER_FRAME
  {
    float4 c_TimeParamters;
  };
  ConstantBuffer<PER_FRAME> cbPerFrame : register(b0);
//---------------------------------------------------------------------------//
  struct PER_VIEWPORT
  {
    float4 c_RenderTargetSize;
  };
  ConstantBuffer<PER_VIEWPORT> cbPerViewport : register(b1);
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
  ConstantBuffer<PER_CAMERA> cbPerCamera : register(b2);
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
  ConstantBuffer<PER_LIGHT> cbPerLight : register(b3);
//---------------------------------------------------------------------------//  
  struct PER_MATERIAL
  {
    float4 c_MatDiffIntensity;
    float4 c_MatSpecIntensity;
  };
  ConstantBuffer<PER_MATERIAL> cbPerMaterial : register(b4);
//---------------------------------------------------------------------------//  
  cbuffer PER_OBJECT : register(b5)
  {
      float4x4 c_WorldMatrix;
      float4x4 c_WorldInverseMatrix;
      float4x4 c_WorldViewMatrix;
      float4x4 c_WorldViewInverseMatrix;
      float4x4 c_WorldViewProjectionMatrix;
      float4x4 c_WorldViewProjectionInverseMatrix;
    
  };
  //ConstantBuffer<PER_OBJECT> cbPerObject : register(b5);
//---------------------------------------------------------------------------//  
#endif // INCLUDE_CONSTANTBUFFER_INCLUDE_