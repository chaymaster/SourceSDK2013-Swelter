#include "BaseVSShader.h"

void DrawWorldPass_World(CBaseShader* pShader, IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr);
void DrawShadowPass_World(CBaseShader* pShader, IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr);


BEGIN_VS_SHADER(SHADER_WORLD, "")
BEGIN_SHADER_PARAMS

END_SHADER_PARAMS

SHADER_INIT
{
	LoadTexture(BASETEXTURE, TEXTUREFLAGS_SRGB);
}

SHADER_INIT_PARAMS()
{
	SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
	SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);
}

SHADER_DRAW
{
//	const int renderStage = pShaderAPI ? pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_RENDER_STAGE) : RENDER_STAGE_UNDEFINED;

	//if (pShaderShadow || renderStage == RENDER_STAGE_WORLDPASS)
//	{
		DrawWorldPass_World(this, params, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr);
//	}
	//else if (pShaderShadow || renderStage == RENDER_STAGE_SHADOWPASS)
	//{
	//	DrawShadowPass_World(this, params, pShaderShadow, pShaderAPI, vertexCompression, pContextDataPtr);
	//}
	//else
	//{
	//	Draw(false);
	//}
}

END_SHADER
