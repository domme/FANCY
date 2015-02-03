//---------------------------------------------------------------------------//
/* Possible Feature-Defines:

*/
//---------------------------------------------------------------------------//

#version 440

 // #include "ConstantBuffer.shader_include"
 //---------------------------------------------------------------------------//
  uniform PER_LAUNCH
  {

  } cbPerLaunch;
//---------------------------------------------------------------------------//
  uniform PER_FRAME
  {

  } cbPerFrame;
//---------------------------------------------------------------------------//
  uniform PER_VIEWPORT
  {
    vec4 c_RenderTargetSize;
  } cbPerViewport;
//---------------------------------------------------------------------------//
  uniform PER_STAGE
  {

  } cbPerStage;
//---------------------------------------------------------------------------//
  uniform PER_CAMERA
  {
    mat4 c_ViewMatrix;
    mat4 c_ProjectionMatrix;
    mat4 c_ViewProjectionMatrix;
    vec4 c_NearFarParameters;
    vec4 c_CameraPosWS;
  } cbPerCamera;
//---------------------------------------------------------------------------//
  uniform PER_LIGHT
  {
    vec4 c_DirLightParameters;
    vec4 c_PointLightParameters;
    vec4 c_SpotLightParameters;
    vec4 c_LightColorIntensity;
    vec4 c_LightPosWS;
    vec4 c_LightPosVS;
  } cbPerLight;
//---------------------------------------------------------------------------//  
  uniform PER_MATERIAL
  {
    vec4 c_MatDiffIntensity;
    vec4 c_MatSpecIntensity;
  } cbPerMaterial;
//---------------------------------------------------------------------------//  
  uniform PER_OBJECT
  {
    mat4 c_WorldMatrix;
    mat4 c_WorldInverseMatrix;
    mat4 c_WorldViewMatrix;
    mat4 c_WorldViewInverseMatrix;
    mat4 c_WorldViewProjectionMatrix;
    mat4 c_WorldViewProjectionInverseMatrix;
  } cbPerObject;
//---------------------------------------------------------------------------//  

//---------------------------------------------------------------------------//
  #if defined (PROGRAM_TYPE_VERTEX)
    out VS_OUT
  #elif defined (PROGRAM_TYPE_FRAGMENT)  
    in VS_OUT
  #endif // PROGRAM_TYPE_VERTEX  
    {
        vec4 color;
    }
  #if defined (PROGRAM_TYPE_VERTEX)      
    vs_out;
  #elif defined (PROGRAM_TYPE_FRAGMENT)
    fs_in;
  #endif  // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_VERTEX)
    layout(location = 0) in vec3 v_position;
    layout(location = 1) in vec3 v_normal;
    layout(location = 2) in vec3 v_tangent;
    layout(location = 3) in vec3 v_bitangent;
    layout(location = 4) in vec2 v_texcoord0;  
    
    void main()
    {
      gl_Position = cbPerObject.c_WorldViewProjectionMatrix * vec4(v_position, 1.0);
      vs_out.color = v_normal;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
    out vec4 color;

    void main()
    {
      color = fs_in.color;
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
