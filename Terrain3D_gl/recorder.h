/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__RECORDER_H__
#define __TUM3D__RECORDER_H__

#include <list>
#include <vector>

#include "terrainLib/Vec.h"

#include "global.h"

class Camera;

struct Sample
{
	double		m_dTime;
	tum3D::Vec3f m_vPosition;
	float		m_fHeading, m_fPitch, m_fRoll, m_fTurn;
};


enum RECORDER_STATE
{
	REC_STATE_IDLE = 0,
	REC_STATE_RECORDING,
	REC_STATE_PLAYING_BACK_PAUSED,
	REC_STATE_PLAYING_BACK,
};

class Recorder
{
public:
	Recorder(Camera* pCamera);
	~Recorder();

	void SetFileName(const char* ptcFileName);
		
	void StartRecording();
	void StopRecording();
	
	void StartPlayingBack(bool bLoop = false, bool bStartPaused = true);
	void ContinuePlayingBack();
	void PausePlayingBack();
	void StopPlayingBack();

	void Update(double dElapsedTime);

	void KeyboardEvent(uchar key);

	double GetTime();
	void SetTime(double dTime);
	
	RECORDER_STATE GetState();

private:
	
	Camera*				m_pCamera;
	char*				m_ptcFileName;

	double				m_dTime;
	std::vector<Sample>	m_samples;
	RECORDER_STATE		m_state;
	uint				m_uiSample;
	float				m_fLength;

	bool				m_bFirstSample;
	bool				m_bLoop;

	uint				m_iNumFrames;

	// TODO: this is not a very good design but avoids duplicate code [1/22/2010 Andreas Kirsch]
	typedef std::list<Recorder*> List; 
	static List s_recorders;
};


inline double Recorder::GetTime()
{
	return m_dTime;
}

inline void Recorder::SetTime(double dTime)
{
	m_dTime = dTime;
}

inline RECORDER_STATE Recorder::GetState()
{
	return m_state;
}


#endif
