/*
#include "BaseVSShader.h"

#include "depth_vs30.inc"
#include "depth_ps30.inc"

void DrawShadowPass_World(CBaseShader* pShader, IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr)
{
	SHADOW_STATE
	{
		pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);
		pShaderShadow->EnableCulling(false);

		DECLARE_STATIC_VERTEX_SHADER(depth_vs30);
		SET_STATIC_VERTEX_SHADER(depth_vs30);
		DECLARE_STATIC_PIXEL_SHADER(depth_ps30);
		SET_STATIC_PIXEL_SHADER(depth_ps30);
	}
	DYNAMIC_STATE
	{
		DECLARE_DYNAMIC_VERTEX_SHADER(depth_vs30);
		SET_DYNAMIC_VERTEX_SHADER(depth_vs30);
		DECLARE_DYNAMIC_PIXEL_SHADER(depth_ps30);
		SET_DYNAMIC_PIXEL_SHADER(depth_ps30);
	}
	pShader->Draw();
}
*/
