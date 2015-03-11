/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/


#ifndef __TUM3D__CAMERA_H__
#define __TUM3D__CAMERA_H__

#include "global.h"

#include "terrainLib/Vec.h"

#ifdef EQUALIZER
#	include <eq/net/dataIStream.h>
#	include <eq/net/dataOStream.h>
#endif

class DataLoader;


enum MOUSE_BUTTON
{
	MOUSE_BUTTON_NONE   = 0,
	MOUSE_BUTTON_LEFT   = 1 << 0,
	MOUSE_BUTTON_MIDDLE = 1 << 1,
	MOUSE_BUTTON_RIGHT  = 1 << 2
};

enum CAM_KEY
{
	CAM_KEY_LEFT = 0,
	CAM_KEY_RIGHT,
	CAM_KEY_UP,
	CAM_KEY_DOWN,
	CAM_KEY_PAGEUP,
	CAM_KEY_PAGEDOWN,
	CAM_KEY_SHIFT,
	CAM_KEY_NUMLEFT,
	CAM_KEY_NUMRIGHT,
	CAM_KEY_NUMUP,
	CAM_KEY_NUMDOWN,
	CAM_KEY_NUMPAGEUP,
	CAM_KEY_NUMPAGEDOWN,
	CAM_KEY_HOME,
	CAM_KEY_END,
	NUM_CAM_KEYS,
	CAM_KEY_UNKNOWN
};

class Camera
{
public:
	Camera();
	Camera(DataLoader* pDataLoader, float fMaxSpeed, float fMinHeight, const RawVec3f vStartPosition, float fStartHeading, float fStartPitch);
	~Camera();

	void Init(float fMaxSpeed, float fMinHeight, const RawVec3f vStartPosition, float fStartHeading, float fStartPitch);

	void Reset();

	void SetPosition(const RawVec3f pvPosition);
	void SetAngles(float fHeading, float fPitch, float fRoll);

	void SetDataLoader(DataLoader* pDataLoader);

	void GetPosition(RawVec3f pvPosition) const;
	void GetAngles(float* pfHeading, float* pfPitch, float* pfRoll) const;

	void GetViewMatrix(RawMat4f pmViewMatrix) const;
	void GetCameraViewOrthogonalFrame(RawVec3f pvViewDirection, RawVec3f pvViewRight, RawVec3f pvViewUp) const;

	// general event interface [9/20/2009 Andreas Kirsch]
	void KeyboardEvent( CAM_KEY key, bool pressed );
	void MouseButtonEvent( MOUSE_BUTTON button, bool pressed, int x, int y );
	void MouseMoveEvent( int x, int y );

	void Update(float fElapsedTime);

	void SetParameters(uint uiViewPortWidth, uint uiViewPortHeight, float fFovy, int iOffsetX = 0, int iOffsetY = 0, const RawMat4f pmHeadMatrix = 0);

	double GetStatCoveredDistance() { return m_dStatCoveredDistance; }

#ifdef EQUALIZER
	void getInstanceData( eq::net::DataOStream& os ) const;
	void applyInstanceData( eq::net::DataIStream& is );
#endif

private:
	void ComputeViewMatrix();

	void ComputeRayDirection(int iMousePositionX, int iMousePositionY, tum3D::Vec3f &pvRayDirection);

	// used for picking, and staying above the ground
	DataLoader*		m_pDataLoader;

	// initial state
	tum3D::Vec3f	m_vStartPosition;
	float			m_fStartHeading, m_fStartPitch;

	// current state - this is all that is serialized
	tum3D::Vec3f	m_vPosition;
	float			m_fHeading, m_fPitch, m_fRoll;

	// computed state, updated by ComputeViewMatrix
	tum3D::Mat4f	m_mViewMatrix;

	tum3D::Vec3f	m_vHorizontalMoveDirection;
	tum3D::Vec3f	m_vHorizontalMoveRight;

	tum3D::Vec3f	m_vMoveDirection;
	tum3D::Vec3f	m_vMoveRight;
	tum3D::Vec3f	m_vMoveUp;

	tum3D::Vec3f	m_vViewDirection;
	tum3D::Vec3f	m_vViewRight;
	tum3D::Vec3f	m_vViewUp;

	// immutable input settings
	float			m_fMaxSpeed;
	float			m_fMinHeight;

	// input settings
	float			m_fKeyboardSpeed;

	// input state
	bool			m_rgbKeyState[NUM_CAM_KEYS];
	uint			m_uiNumKeysPressed;
	uint			m_uiMouseButtonState;
	tum3D::Vec2i	m_vPreviousMousePosition, m_vCurrentMousePosition;
	float			m_fMouseMoveDeltaSmoothedX, m_fMouseMoveDeltaSmoothedY;
	float			m_fSmoothMouseP, m_fSmoothMouseQ;

	// picking parameters
	uint			m_uiViewPortWidth;
	uint			m_uiViewPortHeight;
	int				m_iOffsetX;
	int				m_iOffsetY;
	float			m_fTanHalfFovy;
	float			m_fAspectRatio;
	tum3D::Mat4f	m_mPickingHeadMatrix;

	// picking state
	tum3D::Vec3f	m_vPickedPoint;
	bool			m_bHasPickingStarted;
	float			m_fPickingStartPositionZ;
	float			m_fPickingStartRayDirectionZ;

	// statistic
	double			m_dStatCoveredDistance;
};

#endif
