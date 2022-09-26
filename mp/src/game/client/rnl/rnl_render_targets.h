//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#ifndef RNL_RENDERTARGETS_H_
#define RNL_RENDERTARGETS_H_
#ifdef _WIN32
#pragma once
#endif

#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render targets

// externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CRnLRenderTargets : public CBaseClientRenderTargets
{
	// no networked vars
	DECLARE_CLASS_GAMEROOT( CRnLRenderTargets, CBaseClientRenderTargets );
public:
	virtual void InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig );
	virtual void ShutdownClientRenderTargets();

private:
	
	/*
	//Tony's targets if ever we need radars, flashbangs or nightvision in RnL
	ITexture* CreateRadarTexture( IMaterialSystem* pMaterialSystem );
	ITexture* CreateNVGTexture( IMaterialSystem* pMaterialSystem );
	ITexture* CreateFlashbangTexture( IMaterialSystem* pMaterialSystem );
	*/

	/*
	ITexture* CreateVMDoFTexture( IMaterialSystem* pMaterialSystem );
	*/

	ITexture* CreateVMScopeTexture( IMaterialSystem* pMaterialSystem );
	
private:

	/*
	//Tony's targets if ever we need radars, flashbangs or nightvision in RnL
	CTextureReference		m_RadarTexture;
	CTextureReference		m_NVGTexture;
	CTextureReference		m_FlashbangTexture;
	*/

	/*
	CTextureReference		m_VMDoFTexture;
	*/

	CTextureReference		m_VMScopeTexture;
};

extern CRnLRenderTargets* g_pRnLRenderTargets;


#endif //RNL_RENDERTARGETS_H_