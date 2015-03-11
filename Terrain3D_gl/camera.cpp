/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "camera.h"

#include "terrainLib/dataloader.h"


const uint uiNUM_FRAMES_SMOOTH_MOUSE = 3; // Anzahl der Frames, über die die Mausbewegungen geglättet werden

const float fMIN_MAX_SPEED = 50.0f;
const float fROT_SPEED = 180.0f;

const float fTAN_MIN_PICKING_ANGLE_SQUARED = tan(DEG2RAD(0.15f)) * tan(DEG2RAD(0.15f));


using namespace tum3D;


Camera::Camera()
{
	memset(this, 0, sizeof(Camera));
}

Camera::Camera(DataLoader* pDataLoader, float fMaxSpeed, float fMinHeight, const RawVec3f vStartPosition, float fStartHeading, float fStartPitch)
{
	m_pDataLoader = pDataLoader;

	Init(fMaxSpeed, fMinHeight, vStartPosition, fStartHeading, fStartPitch);
}

void Camera::Init(float fMaxSpeed, float fMinHeight, const RawVec3f vStartPosition, float fStartHeading, float fStartPitch)
{
	m_fMaxSpeed = fMaxSpeed;
	m_fMinHeight = fMinHeight;

	m_vPosition = m_vStartPosition = vStartPosition;
	m_fHeading = m_fStartHeading = fStartHeading;
	m_fPitch = m_fStartPitch = fStartPitch;
	m_fRoll = 0.0f;
	m_fKeyboardSpeed = 500.0f;

	tum3D::identityMat(m_mPickingHeadMatrix);

	ComputeViewMatrix();

	for (uint ui = 0; ui < NUM_CAM_KEYS; ui++)
	{
		m_rgbKeyState[ui] = false;
	}

	m_uiNumKeysPressed = 0;

	m_uiMouseButtonState = 0;

	m_fSmoothMouseP = 1.0f / static_cast<float>(uiNUM_FRAMES_SMOOTH_MOUSE);
	m_fSmoothMouseQ = 1.0f - m_fSmoothMouseP;

	m_dStatCoveredDistance = 0.0;
}


void Camera::SetPosition(const RawVec3f pvPosition)
{
	Vec3f v = Vec3f( pvPosition ) - m_vPosition;
	m_dStatCoveredDistance += v.norm();

	m_vPosition = pvPosition;
	ComputeViewMatrix();
}

void Camera::SetAngles(float fHeading, float fPitch, float fRoll)
{
	m_fHeading = fHeading; m_fPitch = fPitch; m_fRoll = fRoll;
	ComputeViewMatrix();
}

void Camera::SetDataLoader(DataLoader* pDataLoader)
{
	m_pDataLoader = pDataLoader;
}

void Camera::GetPosition(RawVec3f pvPosition) const
{
	if (pvPosition) { memcpy(pvPosition, m_vPosition, 3 * sizeof(float)); }
}

void Camera::GetAngles(float* pfHeading, float* pfPitch, float* pfRoll) const
{
	if (pfHeading) { *pfHeading = m_fHeading; }
	if (pfPitch) { *pfPitch = m_fPitch; }
	if (pfRoll) { *pfRoll = m_fRoll; }
}


void Camera::GetViewMatrix(RawMat4f pmViewMatrix) const
{
	if (pmViewMatrix) { (*(Mat4f*) pmViewMatrix) = m_mViewMatrix; }
}

void Camera::GetCameraViewOrthogonalFrame(RawVec3f pvViewDirection, RawVec3f pvViewRight, RawVec3f pvViewUp) const
{
	if (pvViewDirection) { (*(Vec3f*) pvViewDirection) = m_vViewDirection; }
	if (pvViewRight)     { (*(Vec3f*) pvViewRight)     = m_vViewRight; }
	if (pvViewUp)        { (*(Vec3f*) pvViewUp)        = m_vViewUp; }
}


void Camera::Update(float fElapsedTime)
{
	Vec3f vPreviousPosition = m_vPosition;

	if (m_uiMouseButtonState != 0)
	{
		float fCurrentMouseMoveDeltaX = static_cast<float>(m_vCurrentMousePosition.x() - m_vPreviousMousePosition.x());
		float fCurrentMouseMoveDeltaY = static_cast<float>(m_vCurrentMousePosition.y() - m_vPreviousMousePosition.y());
		m_vPreviousMousePosition = m_vCurrentMousePosition;

		m_fMouseMoveDeltaSmoothedX = m_fSmoothMouseQ * m_fMouseMoveDeltaSmoothedX + m_fSmoothMouseP * fCurrentMouseMoveDeltaX;
		m_fMouseMoveDeltaSmoothedY = m_fSmoothMouseQ * m_fMouseMoveDeltaSmoothedY + m_fSmoothMouseP * fCurrentMouseMoveDeltaY;
	}

	//if (m_iAccumWheelDelta != 0)
	//{
	//	m_fKeyboardSpeed += static_cast<float>(m_iAccumWheelDelta / m_iWheelDeltaPerLine) * (m_fMaxSpeed / 100.0f);
	//	if (m_fKeyboardSpeed < fMIN_MAX_SPEED) { m_fKeyboardSpeed = fMIN_MAX_SPEED; }
	//	else if (m_fKeyboardSpeed > m_fMaxSpeed) { m_fKeyboardSpeed = m_fMaxSpeed; }
	//	
	//	m_iAccumWheelDelta = 0;
	//}

	float fSpeedHeightFactor = sqrt(std::min(std::max(1.0f, m_vPosition.z() / 2000.0f), 25.0f)); // Höhenabhängige Geschwindigkeit

	// Verwendung von Tastatur oder Maus schließt Joystick aus
	if (m_uiNumKeysPressed != 0 || m_uiMouseButtonState != 0)
	{
		// Tastatur und Maus

		// Picking ist exklusiv
		if (m_uiMouseButtonState == MOUSE_BUTTON_RIGHT)
		{
			if (m_bHasPickingStarted)
			{
				Vec3f vRayDirection;
				ComputeRayDirection(m_vCurrentMousePosition.x(), m_vCurrentMousePosition.y(), vRayDirection);
				float f = vRayDirection.x() * vRayDirection.x() + vRayDirection.y() * vRayDirection.y();
				if (sgn(vRayDirection.z()) != sgn(m_fPickingStartRayDirectionZ)
					|| (f != 0.0f && vRayDirection.z() * vRayDirection.z() < fTAN_MIN_PICKING_ANGLE_SQUARED * f))
				{
					if (f == 0.0f)
					{
						vRayDirection.x() = 1.0f;
						vRayDirection.y() = 0.0f;
						f = 1.0f;
					}
					vRayDirection.z() = sgn(m_fPickingStartRayDirectionZ) * sqrt(fTAN_MIN_PICKING_ANGLE_SQUARED * f);
				}
				float t = (m_fPickingStartPositionZ - m_vPickedPoint.z()) / vRayDirection.z();
				assert(t < 0.0f);
				m_vPosition.x() = m_vPickedPoint.x() + t * vRayDirection.x();
				m_vPosition.y() = m_vPickedPoint.y() + t * vRayDirection.y();
				m_vPosition.z() = m_fPickingStartPositionZ;
			}
		}
		else
		{

			float fSpeed;
			if (m_rgbKeyState[CAM_KEY_SHIFT])
			{
				fSpeed = m_fMaxSpeed * fSpeedHeightFactor;
			}
			else
			{
				fSpeed = m_fKeyboardSpeed * fSpeedHeightFactor;
			}

			if (m_rgbKeyState[CAM_KEY_UP])
			{
				m_vPosition += m_vMoveDirection * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_DOWN])
			{
				m_vPosition -= m_vMoveDirection * fSpeed * fElapsedTime;
			}

			if (m_rgbKeyState[CAM_KEY_LEFT])
			{
				m_vPosition -= m_vMoveRight * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_RIGHT])
			{
				m_vPosition += m_vMoveRight * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_NUMLEFT])
			{
				m_vPosition -= m_vHorizontalMoveRight * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_NUMRIGHT])
			{
				m_vPosition += m_vHorizontalMoveRight * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_NUMUP])
			{
				m_vPosition += m_vHorizontalMoveDirection * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_NUMDOWN])
			{
				m_vPosition -= m_vHorizontalMoveDirection * fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_PAGEUP] || m_rgbKeyState[CAM_KEY_NUMPAGEUP])
			{
				m_vPosition.z() += fSpeed * fElapsedTime;
			}
			if (m_rgbKeyState[CAM_KEY_PAGEDOWN] || m_rgbKeyState[CAM_KEY_NUMPAGEDOWN])
			{
				m_vPosition.z() -= fSpeed * fElapsedTime;
			}

			if (m_rgbKeyState[CAM_KEY_HOME])
			{
				m_fKeyboardSpeed += m_fMaxSpeed / 100.0f;
				m_fKeyboardSpeed = std::min(m_fMaxSpeed, m_fKeyboardSpeed);
			}
			if (m_rgbKeyState[CAM_KEY_END])
			{
				m_fKeyboardSpeed -= m_fMaxSpeed / 100.0f;
				m_fKeyboardSpeed = std::max(fMIN_MAX_SPEED, m_fKeyboardSpeed);
			}

			if (m_uiMouseButtonState == MOUSE_BUTTON_LEFT)
			{
				m_fHeading += m_fMouseMoveDeltaSmoothedX / 500.0f * 90.0f;
				m_fPitch += m_fMouseMoveDeltaSmoothedY / 500.0f * 90.0f;
			}



			if (m_fHeading < 0.0f)
			{
				m_fHeading = fmod(m_fHeading, 360.0f) + 360.0f;
			}
			else if (m_fHeading >= 360.0f)
			{
				m_fHeading = fmod(m_fHeading, 360.0f);
			}

			if (m_fPitch < -90.0f)
			{
				m_fPitch = -90.0f;
			}
			else if (m_fPitch > 90.0f)
			{
				m_fPitch = 90.0f;
			}

		}

	}
	
	if(m_pDataLoader) {
		// Sicherstellen, dass die Betrachterposition die Mindesthöhe über der Landschaft einhält
		float fTerrainHeight;
		bool bIsAboveTerrain = m_pDataLoader->GetHeight(m_vPosition.x(), m_vPosition.y(), &fTerrainHeight);
		if (bIsAboveTerrain && m_vPosition.z() < fTerrainHeight + m_fMinHeight)
		{
			m_vPosition.z() = fTerrainHeight + m_fMinHeight;
		}
	}

	ComputeViewMatrix();


	Vec3f v = m_vPosition - vPreviousPosition;
	m_dStatCoveredDistance += v.norm();

}

void Camera::KeyboardEvent( CAM_KEY key, bool pressed ) {
	if (key != CAM_KEY_UNKNOWN)
	{
		if( m_rgbKeyState[key] != pressed )
		{
			m_rgbKeyState[key] = pressed;
			m_uiNumKeysPressed++;
		}
	}
}

void Camera::MouseButtonEvent( MOUSE_BUTTON button, bool pressed, int x, int y ) {
	uint uiPreviousNumKeysPressed = m_uiNumKeysPressed;
	uint uiPreviousMouseButtonState = m_uiMouseButtonState;

	if(pressed) {
		m_uiMouseButtonState |= button;
	}
	else {
		m_uiMouseButtonState &= ~button;
	}

	m_vCurrentMousePosition.x() = x + m_iOffsetX;
	m_vCurrentMousePosition.y() = y + m_iOffsetY;

	if (m_uiMouseButtonState != uiPreviousMouseButtonState)
	{
		if (m_uiMouseButtonState != 0)
		{
			m_vPreviousMousePosition.x() = x + m_iOffsetX;
			m_vPreviousMousePosition.y() = y + m_iOffsetY;
			m_vCurrentMousePosition = m_vPreviousMousePosition;
			m_fMouseMoveDeltaSmoothedX = 0.0f;
			m_fMouseMoveDeltaSmoothedY = 0.0f;
		}

		if (m_uiMouseButtonState == MOUSE_BUTTON_RIGHT && m_pDataLoader)
		{
			Vec3f vRayDirection;
			ComputeRayDirection(m_vCurrentMousePosition.x(), m_vCurrentMousePosition.y(), vRayDirection);

			float f = vRayDirection.x() * vRayDirection.x() + vRayDirection.y() * vRayDirection.y();
			if (f == 0.0f || vRayDirection.z() * vRayDirection.z() >= fTAN_MIN_PICKING_ANGLE_SQUARED * f)
			{
				m_bHasPickingStarted = m_pDataLoader->ComputeRayTerrainIntersection(m_vPosition, vRayDirection, &m_vPickedPoint);
				m_fPickingStartPositionZ = m_vPosition.z();
				m_fPickingStartRayDirectionZ = vRayDirection.z();
			}
			else
			{
				m_bHasPickingStarted = false;
			}
			//outdbg("%f\n", atan(vRayDirection.z / sqrt(f)) / PI * 180.0f);
		}

	}
}

void Camera::MouseMoveEvent( int x, int y ) {
	if (m_uiMouseButtonState != 0)
	{
		m_vCurrentMousePosition.x() = x + m_iOffsetX;
		m_vCurrentMousePosition.y() = y + m_iOffsetY;
	}
}

#ifdef EQUALIZER
void Camera::getInstanceData( eq::net::DataOStream& os ) const {
	os << m_vPosition
	   << m_fHeading << m_fPitch << m_fRoll;
}		

void Camera::applyInstanceData( eq::net::DataIStream& is ) {
	is >> m_vPosition
	   >> m_fHeading >> m_fPitch >> m_fRoll;
	ComputeViewMatrix();
}
#endif


/*************************************************************************************
Berechnung der View-Matrix

Das World-Koordinatensystem ist ein linkshändiges Koordinatensystem, die Landschaft
liegt in der xy-Ebene, die z-Achse zeigt nach oben
Das Kamera-Koordinatensystem ist ein linkshändiges Koordinatensystem, die z-Achse zeigt
in Blickrichtung, die y-Achse zeigt nach oben
Das Kamera-Koordinatensystem geht wie folgt aus dem World-Koordinatensystem hervor:
1. Translation des Ursprungs zur Betrachterposition
2. Heading + Turn: Drehung im Uhrzeigersinn um die z-Achse (Blickrichtung entgegengesetzt zur z-Achse)
3. Pitch: Drehung im Uhrzeigersinn um die y-Achse
4. Roll: Drehung im Uhrzeigersinn um die x-Achse
5. Drehung um 90° im Uhrzeigersinn um die x-Achse
6. Drehung um 90° im Uhrzeigersinn um die y-Achse
Beachte: Für die Transformation der Koordinaten eines Punktes werden die inversen
Transformationen benötigt (in derselben Reihenfolge).
*************************************************************************************/
void Camera::ComputeViewMatrix()
{
	Mat4f mPitchRollHeading, mPitchRoll, m;

	// Berechne Rotationsmatrix für Euler-Winkel-Drehung und Turn
	rotationZMat( DEG2RAD(-m_fHeading), mPitchRollHeading );

	m_vHorizontalMoveDirection = Vec3f( mPitchRollHeading.getRow(0) );
	m_vHorizontalMoveRight = Vec3f( mPitchRollHeading.getRow(1) );

	rotationYMat( DEG2RAD(-m_fPitch), mPitchRoll ); // Pitch
	rotationXMat( DEG2RAD(-m_fRoll), m );

	mPitchRoll = m * mPitchRoll;
	mPitchRollHeading = mPitchRoll * mPitchRollHeading;

	m_vMoveDirection = Vec3f( mPitchRollHeading.getRow( 0 ) );
	m_vMoveRight = Vec3f( mPitchRollHeading.getRow( 1 ) );
	m_vMoveUp = Vec3f( mPitchRollHeading.getRow( 2 ) );

	rotationZMat( DEG2RAD( -m_fHeading ), mPitchRollHeading );
	mPitchRollHeading = mPitchRoll * mPitchRollHeading;

	m_vViewDirection = Vec3f( mPitchRollHeading.getRow( 0 ) );
	m_vViewRight = Vec3f( mPitchRollHeading.getRow( 1 ) );
	m_vViewUp = Vec3f( mPitchRollHeading.getRow( 2 ) );

	// Berechne View-Matrix
	translationMat( -m_vPosition.x(), -m_vPosition.y(), -m_vPosition.z(), m ); // Translation
	m = mPitchRollHeading * m; // Euler-Winkel-Drehung
	m.transpose( m_mViewMatrix );

	tum3D::Mat4f mRotateAxes;
	tum3D::mat4x4(
		0.f, 0.f, -1.f, 0.f,
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
		mRotateAxes
	);
	m_mViewMatrix *= mRotateAxes; // Transformation vom Kamera-KS ins Default-Kamera-KS
}


void Camera::SetParameters(uint uiViewPortWidth, uint uiViewPortHeight, float fFovy, int iOffsetX, int iOffsetY, const RawMat4f pmHeadMatrix)
{
	m_uiViewPortWidth = uiViewPortWidth;
	m_uiViewPortHeight = uiViewPortHeight;
	m_iOffsetX = iOffsetX;
	m_iOffsetY = iOffsetY;
	m_fTanHalfFovy = tan(DEG2RAD(fFovy / 2.0f));
	m_fAspectRatio = static_cast<float>(m_uiViewPortWidth) / static_cast<float>(m_uiViewPortHeight);
	if(pmHeadMatrix)
	{
		tum3D::Mat4f mViewInvMatrix;
		tum3D::invert4x4(m_mViewMatrix, mViewInvMatrix);
		m_mPickingHeadMatrix = m_mViewMatrix * tum3D::Mat4f(pmHeadMatrix) * mViewInvMatrix;
	}
	else
	{
		tum3D::identityMat(m_mPickingHeadMatrix);
	}
}


void Camera::ComputeRayDirection(int iMousePositionX, int iMousePositionY, Vec3f &pvRayDirection)
{
	float fX = 2.0f * static_cast<float>(iMousePositionX) / static_cast<float>(m_uiViewPortWidth) - 1.0f;
	float fY = 1.0f - 2.0f * static_cast<float>(iMousePositionY) / static_cast<float>(m_uiViewPortHeight);
	pvRayDirection = m_vViewDirection + (fX * m_fAspectRatio * m_fTanHalfFovy) * m_vViewRight
		+ (fY * m_fTanHalfFovy) * m_vViewUp;
	tum3D::Vec4f vRayDirectionHead = m_mPickingHeadMatrix * tum3D::Vec4f(pvRayDirection, 0.0f);
	pvRayDirection = vRayDirectionHead.xyz();
}


Camera::~Camera()
{
}

void Camera::Reset() {
	m_vPosition = m_vStartPosition;
	m_fHeading = m_fStartHeading;
	m_fPitch = m_fStartPitch;
	m_fRoll = 0.0f;
}
