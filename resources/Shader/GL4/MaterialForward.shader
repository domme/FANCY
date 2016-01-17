//---------------------------------------------------------------------------//
/* Possible Feature-Defines:
  FEAT_ALBEDO_TEXTURE
*/
//---------------------------------------------------------------------------//

#version 440

 // #include "Shader/TextureSemantics.shader_include"
 #include "Shader/GL4/Common.inc"
 #include "Shader/GL4/ConstantBuffer.shader_include"
 #include "Shader/GL4/Lighting.inc"
 //---------------------------------------------------------------------------//
  #if defined (PROGRAM_TYPE_VERTEX)
    out VS_OUT
  #elif defined (PROGRAM_TYPE_FRAGMENT)  
    in VS_OUT
  #endif // PROGRAM_TYPE_VERTEX  
    {
        vec2 uv;
        vec3 normalWS;
        vec3 posWS;
    }
  #if defined (PROGRAM_TYPE_VERTEX)      
    vs_out;
  #elif defined (PROGRAM_TYPE_FRAGMENT)
    fs_in;
  #endif  // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_VERTEX)
    out gl_PerVertex
    {
      vec4 gl_Position;
    };

    layout(location = 0) in vec3 v_position;
    layout(location = 1) in vec3 v_normal;
    layout(location = 2) in vec3 v_tangent;
    layout(location = 3) in vec3 v_bitangent;
    layout(location = 4) in vec2 v_texcoord0;  

    void main()
    {
      gl_Position = cbPerObject.c_WorldViewProjectionMatrix * vec4(v_position, 1.0);

      vs_out.posWS = (cbPerObject.c_WorldMatrix * vec4(v_position, 1.0)).xyz;
      vs_out.normalWS = normalize((cbPerObject.c_WorldMatrix * vec4(v_normal, 0.0)).xyz);
      vs_out.uv = v_texcoord0;
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
  
  #if defined(FEAT_ALBEDO_TEXTURE)
    layout(binding = 0) uniform sampler2D tex_diffuse;  
  #endif  // FEAT_ALBEDO_TEXTURE
  #if defined (FEAT_NORMAL_MAPPED)
    layout(binding = 1) uniform sampler2D tex_normal;
  #endif  // FEAT_NORMAL_MAPPED
  #if defined (FEAT_SPECULAR) && defined (FEAT_SPECULAR_TEXTURE) 
    layout(binding = 2) uniform sampler2D tex_specular;
  #endif  // (FEAT_SPECULAR) && (FEAT_SPECULAR_TEXTURE)

    out vec4 color;

    void main()
    {
      vec3 albedo = vec3(1.0, 1.0, 1.0);
      #if defined (FEAT_ALBEDO_TEXTURE)
        albedo = texture(tex_diffuse, fs_in.uv).xyz;
      #endif // FEAT_ALBEDO_TEXTURE        

      vec3 lightIntensity = ShadeLight(cbPerLight.c_LightParameters, cbPerLight.c_LightDirWS, fs_in.posWS, normalize(fs_in.normalWS));
      
      color = vec4(albedo, 1.0);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
