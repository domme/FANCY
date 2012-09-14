#ifndef SHADERSEMANTICS_H
#define SHADERSEMANTICS_H

namespace ShaderSemantics
{
	enum Semantic
	{

		//Attribute Semantics
		//////////////////////////////////////////////////////////////////////////
		POSITION = 0,
		NORMAL,
		UV0,
		UV1,
		UV2,
		UV3,
		UV4,
		UV5,
		UV6,
		UV7,
		TANGENT,
		BITANGENT,

		VERTEXCOLOR,

		MAXATTRIBUTES = VERTEXCOLOR,
		//////////////////////////////////////////////////////////////////////////

		//Local Uniforms
		//////////////////////////////////////////////////////////////////////////
		LOCAL_UNIFORMS_BEGIN,

		MODEL,
		MODELI,
		MODELIT,

		MODELWORLD,
		MODELWORLDI,
		MODELWORLDIT,

		MODELWORLDVIEW,
		MODELWORLDVIEWI,
		MODELWORLDVIEWIT,

		MODELWORLDVIEWPROJECTION,
		MODELWORLDVIEWPROJECTIONI,
		MODELWORLDVIEWPROJECTIONIT,

		MODELWORLDLIGHTVIEW,

		TEXTURE,
		TEXTURE_1D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_SIZE,
		
		MATERIALCOLOR,
		AMBIENTREFLECTIVITY,
		DIFFUSEREFLECTIVITY,
		
		BUMPINTENSITY,
		SPECULARCOLOR,
		SPECULAREXPONENT,
		GLOSSINESS,

		VOLMAT_USE_SHADOWS,
		
		SEPERATED_SAMPLING_DIR,
		KERNEL_SIZE,

		LOCAL_UNIFORMS_END,
		//////////////////////////////////////////////////////////////////////////
		
		//Global Uniforms
		//////////////////////////////////////////////////////////////////////////
		GLOBAL_UNIFORMS_BEGIN,

		WORLD,
		WORLDI,
		WORLDIT,

		VIEW,
		VIEWI,
		VIEWIT,

		WORLDVIEW,
		WORLDVIEWI,
		WORLDVIEWIT,

		PROJECTION,
		PROJECTIONI,
		PROJECTIONIT,

		WORLDVIEWPROJECTION,
		WORLDVIEWPROJECTIONI,
		WORLDVIEWPROJECTIONIT,

		TIME,
		
		LIGHTDIRWORLD,
		LIGHTDIRVIEW,

		LIGHTPOSWORLD,
		LIGHTPOSVIEW,

		LIGHTCOLOR,
		LIGHTINTENSITY,
		LIGHTCOLORINTENSITY,

		LIGHTRSTART,
		LIGHTREND,

		LIGHTVIEW,
		LIGHTVIEWI,
		LIGHTPROJ,
		LIGHTPROJI,
				
		FRUSTUM_NEAR,
		FRUSTUM_FAR,
		FRUSTUM_YFOV,

		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		SCREEN_SIZE,
		SCREEN_RATIO,
		SCREEN_TEXTURESTEP,

		
		AMBIENT_LIGHT,
		CLEAR_COLOR,

		HDR_EXPOSURE,
		B_TONEMAPPING_ENABLED,
		B_BLOOM_ENABLED,

		LIGHT_ADAPTION_PERCENTAGE,

		USE_DEBUG_TEXTURES,

		CAMERAPOSWORLD,
		SAMPLING_RATE,
		VOLUME_TEXTURE_SIZE,
		VOLUME_WINDOW_VALUE,

		
		GLOBAL_UNIFORMS_END,
		//////////////////////////////////////////////////////////////////////////

		//MODELWORLDLIGHTVIEW,
		
		NUM,
		UNDEFINED,
		MAX_UV_CHANNELS = TANGENT - NORMAL - 1
	};
}


#endif