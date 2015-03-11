/******************************************************************************

settings.h

Author: Andreas Kirsch

(c) Andreas Kirsch

mailto:kirschan@in.tum.de

*******************************************************************************/

#pragma once

#include "terrainLib/terrainlib.h"
#include "terrainLib/Vec.h"

struct CCDeviceSettings
{
	UINT32 uiSyncInterval;
	BOOL bWindowed;
	UINT uiSampleCount;
	UINT uiSampleQuality;
};

class Settings {
public:
	Settings() :  m_bIsRemoteControl(true), m_bGoFullscreen(false), m_bGoWindow(false), 
		m_bCloseApp(false), m_fEyeDist(0), m_fFocalLength(1500.0f), m_pCamera(0), /*m_pFrustumCulling(0),*/ m_bWireframe(false), m_bBoundingBoxes(false), m_bPageBoundingBoxes(false), m_bStaticMesh(false),
		m_bOcclusionCulling(false),
#ifdef _DEBUG
		m_bRenderStatistics(true),
#else
		m_bRenderStatistics(false),
#endif
#ifdef DEMO
		m_bRenderLogo(true),
#else
		m_bRenderLogo(false),
#endif
		m_bSatModify(false), m_fSaturation(1.0f), m_bDeviceSettingsChanged(false) {}

	bool GetWireframe() const { return m_bWireframe; }
	bool GetBoundingBoxes() const { return m_bBoundingBoxes; }
	bool GetPageBoundingBoxes() const { return m_bPageBoundingBoxes; }
	bool GetStaticMesh() const { return m_bStaticMesh; }
	bool GetOcclusionCulling() const { return m_bOcclusionCulling; }
	bool GetRenderStatistics() const { return m_bRenderStatistics; }
	bool GetRenderLogo() const { return m_bRenderLogo; }
	bool GetSatModify() const { return m_bSatModify; }
	float GetSaturation() const { return m_fSaturation; }

	float GetFocalLength() const {return m_fFocalLength;}
	float GetEyeDist() const {return m_fEyeDist;}

	CCDeviceSettings GetDeviceSettings() const { return m_deviceSettings; }

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
//	void SetFrustumCulling(FrustumCulling* pFrustumCulling) { m_pFrustumCulling = pFrustumCulling; }

	void OnCameraUpdate() {
		m_pCamera->GetViewMatrix( m_mView );
	}

	bool SetWireframe(const bool &bWireframe) {
		m_bWireframe = bWireframe;
		return true;
	}

	bool SetBoundingBoxes(const bool &bBoundingBoxes) {
		m_bBoundingBoxes = bBoundingBoxes;
		return true;
	}

	bool SetPageBoundingBoxes(const bool &bPageBoundingBoxes) {
		m_bPageBoundingBoxes = bPageBoundingBoxes;
		return true;
	}

	bool SetStaticMesh(const bool &bStaticMesh) {
		m_bStaticMesh = bStaticMesh;
		return true;
	}

	bool SetOcclusionCulling(const bool &bOcclusionCulling) {
		m_bOcclusionCulling = bOcclusionCulling;
		return true;
	}

	bool SetRenderStatistics(const bool &bRenderStatistics) {
		m_bRenderStatistics = bRenderStatistics;
		return true;
	}

	bool SetRenderLogo(const bool &bRenderLogo) {
		m_bRenderLogo = bRenderLogo;
		return true;
	}

	bool SetSatModify(const bool &bSatModify) {
		m_bSatModify = bSatModify;
		return true;
	}

	bool SetSaturation(const float &fSaturation) {
		m_fSaturation = fSaturation;
		return true;
	}

	bool SetFocalLength(const float& fFocalLength)
	{
/*
				if (m_pFrustumCulling == 0) return false; // Siehe Kommentar oben
				m_fFocalLength = fFocalLength;
		
				m_pFrustumCulling->SetParameters(m_mProj);*/
		
		return true;
	}

	bool SetEyeDist(const float& fEyeDist)
	{
/*
				if (m_pFrustumCulling == 0) return false; // Siehe Kommentar oben
				m_fEyeDist = fEyeDist;
		
				m_pFrustumCulling->SetParameters(m_mProj);*/
		
		return true;
	}

	bool SetDeviceSettings(const CCDeviceSettings &deviceSettings)
	{
		m_deviceSettings = deviceSettings;
		return true;
	}

protected:
	bool			m_bWireframe;
	bool			m_bBoundingBoxes;
	bool			m_bPageBoundingBoxes;
	bool			m_bStaticMesh;
	bool			m_bOcclusionCulling;
	bool			m_bRenderStatistics;
	bool			m_bRenderLogo;
	bool			m_bSatModify;
	float			m_fSaturation;

public:
	tum3D::Vec2i	m_viTiles;
	int				m_iTileID;
	float			m_fEyeDist;
	float			m_fFocalLength;
	bool			m_bIsRemoteControl;
	bool			m_bLocalOnly;
	bool			m_bGoFullscreen;
	bool            m_bCloseApp;
	bool			m_bGoWindow;
	tum3D::Mat4f	m_mProj;
	tum3D::Mat4f	m_mView;
	float			m_fFOV;
	float			m_fAspect;
	float			m_fZNear;
	float			m_fZFar;
	tum3D::Vec3f	m_vEye;
	tum3D::Vec3f	m_vAt;
	tum3D::Vec3f	m_vUp;

	bool			m_bDeviceSettingsChanged;
	CCDeviceSettings m_deviceSettings;

private:
	Camera*			m_pCamera;

//	FrustumCulling*	m_pFrustumCulling;
};