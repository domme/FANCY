#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "../includes.h"

class Camera;
class Engine;

class CameraController
{
public: 
	static CameraController& getInstance()
	{
		static CameraController instance;
		return instance;
	}

	~CameraController();
	void Init( Camera* pControlledCam );
	void Update();

	const Camera* getCamera() const { return m_pControlledCam; }
	void setControlledCamera( Camera* const newCam ) { m_pControlledCam = newCam; }

private:
	CameraController();
	Camera* m_pControlledCam;
	Engine* m_pEngine;
	int m_iKeyDownListenerID;

	void CheckMouse();
	void onKeyDown( uint16 uKey );
};


#endif