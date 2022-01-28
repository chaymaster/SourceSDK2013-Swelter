#include "BaseVSShader.h"
#include "shaderapi/ishaderapiext.h"
#include "../game/shared/mod_global_defs.h"

#include "world_vs30.inc"
#include "world_ps30.inc"

void DrawWorldPass_World(CBaseShader* pShader, IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr)
{
	SHADOW_STATE
	{
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);

		// Shadow Depth Map
		for (size_t i = 0; i < NUM_SHADOW_CASCADES; ++i)
		{
			pShaderShadow->EnableTexture((Sampler_t)i, true);
			pShaderShadow->EnableSRGBRead((Sampler_t)i, false);
			pShaderShadow->SetShadowDepthFiltering((Sampler_t)i);
		}

		// Albedo
		pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
		pShaderShadow->EnableSRGBRead(SHADER_SAMPLER3, true);

		// Lightmap
		pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);
		pShaderShadow->EnableSRGBRead(SHADER_SAMPLER4, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE);

		pShaderShadow->EnableSRGBWrite(true);

		DECLARE_STATIC_VERTEX_SHADER(world_vs30);
		SET_STATIC_VERTEX_SHADER(world_vs30);
		DECLARE_STATIC_PIXEL_SHADER(world_ps30);
		SET_STATIC_PIXEL_SHADER(world_ps30);
	}
	DYNAMIC_STATE
	{
		if (!pShader->UsingEditor(params))
		{
			for (size_t i = 0; i < NUM_SHADOW_CASCADES; ++i)
			{
				pShader->BindTexture((Sampler_t)i, shaderapi->GetOrthoDepthTexture(i));
				VMatrix worldToTexture = shaderapi->GetShadowCascadeState(i).matWorldToTexture;
				pShaderAPI->SetPixelShaderConstant(i * 4, worldToTexture.Base(), 4);
			}
		}

		pShader->BindTexture(SHADER_SAMPLER3, BASETEXTURE);
		pShaderAPI->BindStandardTexture(SHADER_SAMPLER4, TEXTURE_LIGHTMAP_BUMPED);

		VMatrix matViewProj, matPrevViewProj;
		shaderapi->GetViewProjectionMatrix(matViewProj, matPrevViewProj);
		pShaderAPI->SetVertexShaderConstant(48, matViewProj.Base(), 4);
		pShaderAPI->SetVertexShaderConstant(52, matPrevViewProj.Base(), 4);

		// Modulation color for lightmaps...
		float color[4] = { 1.0, 1.0, 1.0, 1.0 };
		float flLScale = pShaderAPI->GetLightMapScaleFactor();
		color[0] *= flLScale;
		color[1] *= flLScale;
		color[2] *= flLScale;
		pShaderAPI->SetPixelShaderConstant(13, color);
				
		DECLARE_DYNAMIC_VERTEX_SHADER(world_vs30);
		SET_DYNAMIC_VERTEX_SHADER(world_vs30);
		DECLARE_DYNAMIC_PIXEL_SHADER(world_ps30);
		SET_DYNAMIC_PIXEL_SHADER(world_ps30);
	}
	pShader->Draw();
}
