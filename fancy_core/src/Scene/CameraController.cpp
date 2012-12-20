#include "CameraController.h"
#include "../Engine.h"
#include "Camera.h"
#include "SceneNode.h"
#include "../Services/NameRegistry.h"


CameraController::CameraController() : 
m_pControlledCam( NULL ),
m_pEngine( NULL )
{

}

CameraController::~CameraController()
{
	m_pEngine->UnregisterKeyDownListener( m_iKeyDownListenerID );
}


void CameraController::Init( Camera* pControlledCam )
{
	m_pControlledCam = pControlledCam;
	m_pEngine = &Engine::GetInstance();

	m_iKeyDownListenerID = m_pEngine->RegisterKeyDownListener<CameraController>( this, &CameraController::onKeyDown );
}

void CameraController::Update()
{
	CheckMouse();
}

void CameraController::onKeyDown( uint16 uKey )
{
	/*float fCurrMovementSpeed = 15.0f * m_pEngine->GetMovementMul();

	switch( uKey )
	{
		case SDLK_w:
			m_pControlledCam->m_v3Position += ( m_pControlledCam->m_v3View * fCurrMovementSpeed );
		break;
	
		case SDLK_s:
			m_pControlledCam->m_v3Position -= ( m_pControlledCam->m_v3View * fCurrMovementSpeed );
		break;

		case SDLK_a:
			m_pControlledCam->m_v3Position -= ( m_pControlledCam->m_v3Side * fCurrMovementSpeed );
		break;

		case SDLK_d:
			m_pControlledCam->m_v3Position += ( m_pControlledCam->m_v3Side * fCurrMovementSpeed );
		break;
	}*/
}

void CameraController::CheckMouse()
{
	/*DomEngine::Input::SMouseInformationSet* currentMState = DomEngine::Input::MouseState::GetCurrentMouseState();

	if( ( abs( currentMState->m_relativeMouseMovement.x ) < 0.1f && abs( currentMState->m_relativeMouseMovement.y ) < 0.1f ) )
	{
		//no mouse-movement!
		return;
	}

	float angleX = -currentMState->m_relativeMouseMovement.x * ( 1.0f / ( 1 + m_pEngine->GetMovementMul() ) );
	float angleY = -currentMState->m_relativeMouseMovement.y * ( 1.0f / ( 1 + m_pEngine->GetMovementMul() ) );

	if( !currentMState->IsMouseButtonsPressed( DomEngine::Input::DOM_MOUSEB_RIGHT ) )
	{
		m_pControlledCam->RotateViewQuat( angleY, m_pControlledCam->m_v3Side );
		m_pControlledCam->RotateViewQuat( angleX, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	}

	//handle Spotlight rotation
	else
	{
		SceneNode* pSpotNode = NodeRegistry::getInstance().getObject( "spotNode" );
		if( pSpotNode )
			pSpotNode->rotate( angleY / 100, glm::vec3( 0.0f, 0.0f, 1.0f ) );
	}*/
}

