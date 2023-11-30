#include "cbase.h"
#include "screenspaceeffects.h"
#include "rendertexture.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "cdll_client_int.h"
#include "materialsystem/itexture.h"
#include "keyvalues.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_rnl_player.h"

class CVisionBlurEffect : public IScreenSpaceEffect
{
public:
	CVisionBlurEffect( void ) : 
		m_pBlurTexture( NULL ), 
		m_flDuration( 0.0f ), 
		m_flFinishTime( 0.0f ), 
		m_flScale( 1.0f ),
		m_flSpeed( 1.0f ),
		m_bUpdated( false ) {}

	void Init( void ) OVERRIDE;
	void Shutdown( void ) OVERRIDE {}
	void SetParameters( KeyValues *params ) OVERRIDE;
	void Enable( bool bEnable ) OVERRIDE {};
	bool IsEnabled( ) OVERRIDE { return true; }

	void Render( int x, int y, int w, int h ) OVERRIDE;

private:
	ITexture	*m_pBlurTexture;
	float		m_flDuration;
	float		m_flBluriness;
	float		m_flFinishTime;
	float		m_flNextUpdate;
	float		m_flScale;
	float		m_flSpeed;
	bool		m_bUpdated;
};

ADD_SCREENSPACE_EFFECT( CVisionBlurEffect, rnl_visionblur );

CLIENTEFFECT_REGISTER_BEGIN( PrecacheVisionBlurEffect )
CLIENTEFFECT_MATERIAL( "effects/visionblur" )
CLIENTEFFECT_REGISTER_END()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CVisionBlurEffect::Init( void ) 
{
	m_pBlurTexture = NULL;
	m_flDuration = 0.0f;
	m_flFinishTime = 0.0f;
	m_flScale = 1.0f;
	m_flSpeed = 1.0f;
	m_bUpdated = false;
}

//------------------------------------------------------------------------------
// Purpose: Pick up changes in our parameters
//------------------------------------------------------------------------------
void CVisionBlurEffect::SetParameters( KeyValues *params )
{
	if( params->FindKey( "duration" ) )
	{
		m_flDuration = params->GetFloat( "duration" );
		if( m_flDuration > 0.0 )
		{
			if( m_flFinishTime < (gpGlobals->curtime + m_flDuration) )
				m_flFinishTime = gpGlobals->curtime + m_flDuration;
		}
		else
			m_flFinishTime = gpGlobals->curtime - 2.0f;

		m_bUpdated = true;
	}

	if( params->FindKey( "bluriness" ) )
		m_flBluriness = params->GetFloat( "bluriness" );
	else
		m_flBluriness = 0.01;

	if( params->FindKey( "scale" ) )
		m_flScale = params->GetFloat( "scale" );
	else
		m_flScale = 1.0f;

	if( params->FindKey( "speed" ) )
		m_flSpeed = params->GetFloat( "speed" );
	else
		m_flSpeed = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Render the effect
//-----------------------------------------------------------------------------
void CVisionBlurEffect::Render( int x, int y, int w, int h )
{
	// Make sure we're ready to play this effect
	if ( m_flFinishTime < gpGlobals->curtime )
	{
		if( IsEnabled() )
		{
			g_pScreenSpaceEffects->DisableScreenSpaceEffect( "rnl_visionblur" );
		}
		return;
	}

	IMaterial *pMaterial = materials->FindMaterial( "effects/visionblur", TEXTURE_GROUP_CLIENT_EFFECTS, true );
	if ( pMaterial == NULL )
		return;

	//bool bResetBaseFrame = m_bUpdated;

	// Set ourselves to the proper rendermode
	IMatRenderContext* pContext = materials->GetRenderContext();
	if( !pContext )
		return;

	pContext->MatrixMode( MATERIAL_VIEW );
	pContext->PushMatrix();
	pContext->LoadIdentity();
	pContext->MatrixMode( MATERIAL_PROJECTION );
	pContext->PushMatrix();
	pContext->LoadIdentity();

	// Get our current view
	if ( m_pBlurTexture == NULL )
	{
		m_pBlurTexture = GetPowerOfTwoFrameBufferTexture();
	}

	// Draw the texture if we're using it
	if ( m_pBlurTexture != NULL )
	{
		bool foundVar;
		IMaterialVar* pBaseTextureVar = pMaterial->FindVar( "$basetexture", &foundVar, false );
		IMaterialVar* pAlpha = pMaterial->FindVar( "$alpha", &foundVar, false );

		if( pAlpha )
		{
			pAlpha->SetFloatValue( 0.34f );
		}


		if( m_flNextUpdate < gpGlobals->curtime )
		{
			CRnLPlayer* pPlayer = CRnLPlayer::GetLocalRnLPlayer();
			if( pPlayer )
			{
				QAngle offset( random->RandomFloat( -5.0, 5.0 ) * cos( gpGlobals->curtime * 2.0f), random->RandomFloat( -5.0, 5.0 ) * sin( gpGlobals->curtime * 2.0f), 0.0f );
				offset.x *= m_flBluriness;
				offset.y *= m_flBluriness;
				pPlayer->AdjustViewAngles( offset );
			}
			m_flNextUpdate = gpGlobals->curtime + m_flBluriness;
		}

		// Save off this pass
		Rect_t srcRect;
		srcRect.x = x;
		srcRect.y = y;
		srcRect.width = w;
		srcRect.height = h;
		pBaseTextureVar->SetTextureValue( m_pBlurTexture );

		pContext->CopyRenderTargetToTextureEx( m_pBlurTexture, 0, &srcRect, NULL );
		pContext->SetFrameBufferCopyTexture( m_pBlurTexture );
		m_bUpdated = false;

		float flCurrentTime = gpGlobals->curtime * m_flSpeed; // modified for speed

		float flEffectPerc = clamp( m_flScale * ( ( m_flFinishTime - gpGlobals->curtime ) / m_flDuration ) , 0.0f, 1.0f );
		float vX = ( flEffectPerc * 48.0f ) * sin( flCurrentTime * 10.0f ) * cos( flCurrentTime * 2.0f );
		float vY = ( flEffectPerc * 32.0f ) * cos( flCurrentTime * 10.0f ) * sin( flCurrentTime * 2.0f );

		pContext->DrawScreenSpaceRectangle( pMaterial, x-vX, y-vY, w, h,
			0, 0, m_pBlurTexture->GetActualWidth()-1, m_pBlurTexture->GetActualHeight()-1, 
			m_pBlurTexture->GetActualWidth(), m_pBlurTexture->GetActualHeight() );

		pContext->DrawScreenSpaceRectangle( pMaterial, x+vX, y+vY, w, h,
			0, 0, m_pBlurTexture->GetActualWidth()-1, m_pBlurTexture->GetActualHeight()-1, 
			m_pBlurTexture->GetActualWidth(), m_pBlurTexture->GetActualHeight() );

		pContext->DrawScreenSpaceRectangle( pMaterial, x-vX*2, y-vY*2, w, h,
			0, 0, m_pBlurTexture->GetActualWidth()-1, m_pBlurTexture->GetActualHeight()-1, 
			m_pBlurTexture->GetActualWidth(), m_pBlurTexture->GetActualHeight() );

		pContext->DrawScreenSpaceRectangle( pMaterial, x+vX*2, y+vY*2, w, h,
			0, 0, m_pBlurTexture->GetActualWidth()-1, m_pBlurTexture->GetActualHeight()-1, 
			m_pBlurTexture->GetActualWidth(), m_pBlurTexture->GetActualHeight() );

		// Save off this pass
		srcRect.x = x;
		srcRect.y = y;
		srcRect.width = w;
		srcRect.height = h;
		pBaseTextureVar->SetTextureValue( m_pBlurTexture );

		pContext->CopyRenderTargetToTextureEx( m_pBlurTexture, 0, &srcRect, NULL );
	}

	// Restore our state
	pContext->MatrixMode( MATERIAL_VIEW );
	pContext->PopMatrix();
	pContext->MatrixMode( MATERIAL_PROJECTION );
	pContext->PopMatrix();
}