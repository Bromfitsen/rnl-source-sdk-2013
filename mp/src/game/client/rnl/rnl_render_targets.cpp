//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "rnl_render_targets.h"
#include "materialsystem\imaterialsystem.h"
#include "rendertexture.h"

/*
//Tony's targets if ever we need radars, flashbangs or nightvision in RnL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ITexture* CRnLRenderTargets::CreateRadarTexture( IMaterialSystem* pMaterialSystem )
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Radar",
		256, 256, RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		0 );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ITexture* CRnLRenderTargets::CreateNVGTexture( IMaterialSystem* pMaterialSystem )
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_NVG",
		512, 512, RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED, 
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ITexture* CRnLRenderTargets::CreateFlashbangTexture( IMaterialSystem* pMaterialSystem )
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Flashbang",
		512, 512, RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED, 
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		0 );
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ITexture* CRnLRenderTargets::CreateVMDoFTexture( IMaterialSystem* pMaterialSystem )
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_VMDoF",
		128, 128, RT_SIZE_HDR,
		pMaterialSystem->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SEPARATE );
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ITexture* CRnLRenderTargets::CreateVMScopeTexture( IMaterialSystem* pMaterialSystem )
{
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_VMScope",
		128, 128, RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SEPARATE );
}

//-----------------------------------------------------------------------------
// Purpose: InitClientRenderTargets, interface called by the engine at material system init in the engine
// Input  : pMaterialSystem - the interface to the material system from the engine (our singleton hasn't been set up yet)
//			pHardwareConfig - the user's hardware config, useful for conditional render targets setup
//-----------------------------------------------------------------------------
void CRnLRenderTargets::InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig )
{
	// Water effects & camera from the base class (standard HL2 targets)
	BaseClass::InitClientRenderTargets( pMaterialSystem, pHardwareConfig );


	/*
	//Tony's targets if ever we need radars, flashbangs or nightvision in RnL
	m_RadarTexture.Init( CreateRadarTexture( pMaterialSystem ) );
	m_NVGTexture.Init( CreateNVGTexture( pMaterialSystem ) );
	m_FlashbangTexture.Init( CreateFlashbangTexture( pMaterialSystem ) );
	*/

	/*
	m_VMDoFTexture.Init( CreateVMDoFTexture( pMaterialSystem ) );
	*/
	m_VMScopeTexture.Init( CreateVMScopeTexture( pMaterialSystem ) );
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown client render targets. This gets called during shutdown in the engine
// Input  :  - 
//-----------------------------------------------------------------------------
void CRnLRenderTargets::ShutdownClientRenderTargets()
{

	// Clean up standard HL2 RTs (camera and water)
	BaseClass::ShutdownClientRenderTargets();

	/*
	//Tony's targets if ever we need radars, flashbangs or nightvision in RnL
	m_RadarTexture.Shutdown(true);
	m_NVGTexture.Shutdown(true);
	m_FlashbangTexture.Shutdown(true);
	*/

	/*
	m_VMDoFTexture.Shutdown( true );
	*/
	m_VMScopeTexture.Shutdown( true );
}

//Tony; add the interface!
static CRnLRenderTargets g_RnLRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CRnLRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_RnLRenderTargets );
CRnLRenderTargets* g_pRnLRenderTargets = &g_RnLRenderTargets;