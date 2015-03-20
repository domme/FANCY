//---------------------------------------------------------------------------//
/* Possible Feature-Defines:
  FEAT_ALBEDO_TEXTURE
*/
//---------------------------------------------------------------------------//

#version 440

 // #include "Shader/TextureSemantics.shader_include"
 #include "Shader/ConstantBuffer.shader_include"
 //---------------------------------------------------------------------------//
  #if defined (PROGRAM_TYPE_VERTEX)
    out VS_OUT
  #elif defined (PROGRAM_TYPE_FRAGMENT)  
    in VS_OUT
  #endif // PROGRAM_TYPE_VERTEX  
    {
        vec2 uv;
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
      color = texture(tex_diffuse, fs_in.uv);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
