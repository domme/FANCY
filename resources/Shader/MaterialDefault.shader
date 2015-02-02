//---------------------------------------------------------------------------//
/* Possible Feature-Defines:

*/
//---------------------------------------------------------------------------//

#version 440

#if defined(PROGRAM_TYPE_VERTEX)
  
    struct in SVertexIn
    {
      vec3 v_position;
      vec3 v_normal;
      vec3 v_tangent;
      vec3 v_bitangent;
      vec2 v_texcoord0;  
    } vertexIn;

    struct out SVertexOut
    {

    } vertexOut;
    
    void main()
    {
      
    }

#endif // PROGRAM_TYPE_VERTEX



#if defined(PROGRAM_TYPE_FRAGMENT)


#endif // PROGRAM_TYPE_FRAGMENT
