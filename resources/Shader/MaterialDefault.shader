//---------------------------------------------------------------------------//
/* Possible Feature-Defines:

*/
//---------------------------------------------------------------------------//

#version 440

 #include "Shader/ConstantBuffer.shader_include"

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
      // mat4 worldViewProj = mat4(vec4(1.8106, 0, 0, 0), vec4(0, 2.4142, 0, 0), vec4(0, 0, -1.002, -1), vec4(0, 0, 8.01801, 10));
      mat4 worldViewProj = cbPerObject.c_WorldViewProjectionMatrix;

      gl_Position = worldViewProj * vec4(v_position, 1.0);
      vs_out.color = abs(cbPerObject.c_WorldMatrix[3]);
    }
  #endif // PROGRAM_TYPE_VERTEX
//---------------------------------------------------------------------------//
  #if defined(PROGRAM_TYPE_FRAGMENT)
    out vec4 color;

    void main()
    {
      color = vec4(fs_in.color.xyz, 1.0);
    }
  #endif // PROGRAM_TYPE_FRAGMENT
//---------------------------------------------------------------------------//  
