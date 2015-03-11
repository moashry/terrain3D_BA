/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "recorder.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <windows.h> // needed for open/save file dialogs

#include "global.h"
#include "camera.h"

Recorder::List Recorder::s_recorders;

Recorder::Recorder(Camera* pCamera)
{
	m_ptcFileName = 0;
	m_state = REC_STATE_IDLE;

	m_dTime = 0.0;

	m_pCamera = pCamera;

	m_bLoop = false;

	s_recorders.push_back(this);
}


Recorder::~Recorder()
{
	s_recorders.remove(this);

	if (m_ptcFileName) { delete[] m_ptcFileName; }
}

void Recorder::SetFileName(const char* ptcFileName)
{
	if (m_ptcFileName) { delete[] m_ptcFileName; }
	size_t stCount = strlen(ptcFileName) + 1;
	m_ptcFileName = new char[stCount];
	strncpy_s(m_ptcFileName, stCount * sizeof(char), ptcFileName, stCount);
}


void Recorder::StartRecording()
{
	assert(m_state == REC_STATE_IDLE);

	m_state = REC_STATE_RECORDING;

	m_samples.clear();

	m_dTime = 0.0;
	m_iNumFrames = 0;
	m_bFirstSample = true;
}


void Recorder::StopRecording()
{
	assert(m_state == REC_STATE_RECORDING);

	m_state = REC_STATE_IDLE;

	FILE* hFile;
	errno_t err = fopen_s(&hFile, m_ptcFileName, "wb");
	if (err != 0)
	{
		printf_s("Could not create file %s\n" __LOCATION__, m_ptcFileName);
		exit(-1);
	}
	uint uiNumSamples = static_cast<uint>(m_samples.size());
	fwrite(&uiNumSamples, sizeof(uint), 1, hFile);
	for (uint uiSample = 0; uiSample < uiNumSamples; uiSample++)
	{
		fwrite(&m_samples[uiSample].m_dTime, sizeof(double), 1, hFile);
		fwrite(&m_samples[uiSample].m_vPosition.x(), sizeof(float), 1, hFile);
		fwrite(&m_samples[uiSample].m_vPosition.y(), sizeof(float), 1, hFile);
		fwrite(&m_samples[uiSample].m_vPosition.z(), sizeof(float), 1, hFile);
		fwrite(&m_samples[uiSample].m_fHeading, sizeof(float), 1, hFile);
		fwrite(&m_samples[uiSample].m_fPitch, sizeof(float), 1, hFile);
		fwrite(&m_samples[uiSample].m_fRoll, sizeof(float), 1, hFile);
		fwrite(&m_samples[uiSample].m_fTurn, sizeof(float), 1, hFile);
	}

	fclose(hFile);
}


void Recorder::StartPlayingBack(bool bLoop, bool bStartPaused)
{
	assert(m_state == REC_STATE_IDLE);

	m_bLoop = bLoop;

	FILE* hFile;
	errno_t err = fopen_s(&hFile, m_ptcFileName, "rb");
	if (err != 0)
	{
		printf_s("Could not open file %s\n" __LOCATION__, m_ptcFileName);
		exit(-1);
	}
	uint uiNumSamples;
	fread_s(&uiNumSamples, sizeof(uint), sizeof(uint), 1, hFile);
	m_samples.resize(uiNumSamples);
	for (uint uiSample = 0; uiSample < uiNumSamples; uiSample++)
	{
		memset(&m_samples[uiSample], 0, sizeof(Sample));
		fread_s(&m_samples[uiSample].m_dTime, sizeof(double), sizeof(double), 1, hFile);
		fread_s(&m_samples[uiSample].m_vPosition.x(), sizeof(float), sizeof(float), 1, hFile);
		fread_s(&m_samples[uiSample].m_vPosition.y(), sizeof(float), sizeof(float), 1, hFile);
		fread_s(&m_samples[uiSample].m_vPosition.z(), sizeof(float), sizeof(float), 1, hFile);
		fread_s(&m_samples[uiSample].m_fHeading, sizeof(float), sizeof(float), 1, hFile);
		fread_s(&m_samples[uiSample].m_fPitch, sizeof(float), sizeof(float), 1, hFile);
		fread_s(&m_samples[uiSample].m_fRoll, sizeof(float), sizeof(float), 1, hFile);
		fread_s(&m_samples[uiSample].m_fTurn, sizeof(float), sizeof(float), 1, hFile);
	}
	fclose(hFile);

	m_dTime = 0.0;
	m_iNumFrames = 0;

	// Kamera setzen
	m_state = REC_STATE_PLAYING_BACK;
	m_bFirstSample = true;
	m_uiSample = 0;

	if (bStartPaused)
	{
		Update(0.0);

		m_state = REC_STATE_PLAYING_BACK_PAUSED;
		m_bFirstSample = true;
		m_uiSample = 0;
	}
}


void Recorder::ContinuePlayingBack()
{
	assert(m_state == REC_STATE_PLAYING_BACK_PAUSED);

	m_state = REC_STATE_PLAYING_BACK;
}


void Recorder::PausePlayingBack()
{
	assert(m_state == REC_STATE_PLAYING_BACK);

	m_state = REC_STATE_PLAYING_BACK_PAUSED;
}


void Recorder::StopPlayingBack()
{
	assert(m_state == REC_STATE_PLAYING_BACK || m_state == REC_STATE_PLAYING_BACK_PAUSED);

	m_state = REC_STATE_IDLE;

	printf_s( "Total time: %g; Frame count: %u --- Average FPS: %g\n", m_dTime, m_iNumFrames, m_iNumFrames / m_dTime );
}


void Recorder::Update(double dElapsedTime)
{
	if (!m_bFirstSample)
	{
		m_dTime += dElapsedTime;
		m_iNumFrames++;
	}
	else
	{
		m_bFirstSample = false;
	}

	if (m_state == REC_STATE_RECORDING)
	{
		Sample sample;
		memset(&sample, 0, sizeof(Sample));
		sample.m_dTime = m_dTime;
		m_pCamera->GetPosition(sample.m_vPosition);
		m_pCamera->GetAngles(&sample.m_fHeading, &sample.m_fPitch, &sample.m_fRoll);
		m_samples.push_back(sample);
	}
	else if (m_state == REC_STATE_PLAYING_BACK)
	{
		while (m_uiSample < m_samples.size() && m_samples[m_uiSample].m_dTime <= m_dTime)
		{
			m_uiSample++;
		}
		// Invariante: m_uiSample == m_samples.size() || m_dTime < m_samples[m_uiSample].m_dTime
		// m_dTime < m_samples[m_uiSample].m_dTime => m_uiSample > 0 && m_samples[m_uiSample - 1].m_dTime <= m_dTime
		// (Da m_dTime >= 0 und m_samples[0].m_dTime == 0)

		if (m_uiSample == m_samples.size())
		{
			//m_pCamera->SetPosition(&m_samples[m_uiSample - 1].m_vPosition);
			//m_pCamera->SetAngles(m_samples[m_uiSample - 1].m_fHeading, m_samples[m_uiSample - 1].m_fPitch, m_samples[m_uiSample - 1].m_fRoll, m_samples[m_uiSample - 1].m_fTurn);

			if (!m_bLoop)
			{
				StopPlayingBack();
			}
			else
			{
				m_dTime = 0.0;
				m_uiSample = 0;
				m_iNumFrames = 0;
				// TODO: output states here, too [10/12/2009 Andreas Kirsch]
				while (m_uiSample < m_samples.size() && m_samples[m_uiSample].m_dTime <= m_dTime)
				{
					m_uiSample++;
				}
				if (m_uiSample == m_samples.size())
				{
					StopPlayingBack();
				}
			}
		}
		else
		{
			// Interpoliere zwischen den beiden Samples
			float f = static_cast<float>((m_dTime - m_samples[m_uiSample - 1].m_dTime) / (m_samples[m_uiSample].m_dTime - m_samples[m_uiSample - 1].m_dTime));
						
			// *** Position ***
			tum3D::Vec3f vPosition;
			vPosition.x() = (1.0f - f) * m_samples[m_uiSample - 1].m_vPosition.x() + f * m_samples[m_uiSample].m_vPosition.x();
			vPosition.y() = (1.0f - f) * m_samples[m_uiSample - 1].m_vPosition.y() + f * m_samples[m_uiSample].m_vPosition.y();
			vPosition.z() = (1.0f - f) * m_samples[m_uiSample - 1].m_vPosition.z() + f * m_samples[m_uiSample].m_vPosition.z();
			
			// *** Heading ***
			float fHeadingA = m_samples[m_uiSample - 1].m_fHeading;
			float fHeadingB = m_samples[m_uiSample].m_fHeading;
			// Falls die "k�rzere" Drehung von < 360� �ber 0� nach >= 0� f�hrt, oder umgekehrt
			// (Bei 180� wird stets im positiven Sinn gedreht)
			if (fHeadingB - fHeadingA <= -180.0f)
			{
				fHeadingA -= 360.0f;
			}
			else if (180.0f < fHeadingB - fHeadingA)
			{
				fHeadingB -= 360.0f;
			}
			// Sonst: -180.0f < fHeadingB - fHeadingA <= 180.0f
			float fHeading = (1.0f - f) * fHeadingA + f * fHeadingB;
			
			// *** Pitch ***
			float fPitch = (1.0f - f) * m_samples[m_uiSample - 1].m_fPitch + f * m_samples[m_uiSample].m_fPitch;
			
			// *** Roll ***
			float fRoll = (1.0f - f) * m_samples[m_uiSample - 1].m_fRoll + f * m_samples[m_uiSample].m_fRoll;
			
			m_pCamera->SetPosition(vPosition);
			m_pCamera->SetAngles(fHeading, fPitch, fRoll);

		}

	}

}


static bool OpenFileNameDialog(char* ptcFileName, DWORD dwMaxLength, const char* ptcFilter, DWORD dwFilterIndex)
{
	/*
	bool bToggleFullScreen = !DXUTIsWindowed();
	if (bToggleFullScreen)
	{
	DXUTToggleFullScreen();
	}
	*/

	OPENFILENAMEA openFileName;
	ZeroMemory(&openFileName, sizeof(OPENFILENAME));
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = /*DXUTGetHWND()*/ 0;
	openFileName.lpstrFilter = ptcFilter;
	openFileName.nFilterIndex = dwFilterIndex;
	openFileName.lpstrFile = ptcFileName;
	ptcFileName[0] = 0;
	openFileName.nMaxFile = dwMaxLength;
	openFileName.Flags = OFN_FILEMUSTEXIST;
	BOOL bResult = GetOpenFileNameA(&openFileName);

	/*
	if (bToggleFullScreen)
	{
	DXUTToggleFullScreen();
	}*/


	return bResult != 0;
}


static bool SaveFileNameDialog(char* ptcFileName, DWORD dwMaxLength, const char* ptcFilter, DWORD dwFilterIndex)
{
	/*
	bool bToggleFullScreen = !DXUTIsWindowed();
	if (bToggleFullScreen)
	{
	DXUTToggleFullScreen();
	}*/


	OPENFILENAMEA openFileName;
	ZeroMemory(&openFileName, sizeof(OPENFILENAME));
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = /*DXUTGetHWND()*/ 0;
	openFileName.lpstrFilter = ptcFilter;
	openFileName.nFilterIndex = dwFilterIndex;
	openFileName.lpstrFile = ptcFileName;
	ptcFileName[0] = 0;
	openFileName.nMaxFile = dwMaxLength;
	BOOL bResult = GetSaveFileNameA(&openFileName);

	/*
	if (bToggleFullScreen)
	{
	DXUTToggleFullScreen();
	}*/


	return bResult != 0;
}

void Recorder::KeyboardEvent( uchar key ) {
	switch( key ) {
	case 'R':
		{
			if (m_state == REC_STATE_IDLE)
			{
				char atcFileName[MAX_STRING_SIZE];
				//_tcscpy_s(atcFileName, MAX_STRING_SIZE, "benchmark.rec");
				bool bResult = SaveFileNameDialog(atcFileName, MAX_STRING_SIZE, "All Files (*.*)\0*.*\0Recorded Flights (*.rec)\0*.rec\0", 2);
				if (bResult)
				{
					SetFileName(atcFileName);
					StartRecording();
				}
			}
			else if (m_state == REC_STATE_RECORDING)
			{
				StopRecording();
			}
			break;
		}
	case 'T':
		{
			if (m_state == REC_STATE_IDLE)
			{
				char atcFileName[MAX_STRING_SIZE];
				//_tcscpy_s(atcFileName, MAX_STRING_SIZE, "1.rec"); BOOL bResult = TRUE;
				bool bResult = OpenFileNameDialog(atcFileName, MAX_STRING_SIZE, "All Files (*.*)\0*.*\0Recorded Flights (*.rec)\0*.rec\0", 2);
				if (bResult)
				{
					SetFileName(atcFileName);
					StartPlayingBack();
				}
			}
			else if (m_state == REC_STATE_PLAYING_BACK || m_state == REC_STATE_PLAYING_BACK_PAUSED)
			{
				StopPlayingBack();
			}
			break;
		}
	case ' ':
		{
			if (m_state == REC_STATE_PLAYING_BACK_PAUSED)
			{
				ContinuePlayingBack();
			}
			else if (m_state == REC_STATE_PLAYING_BACK)
			{
				PausePlayingBack();
			}
			break;
		}
	// start playing all loaded records [1/22/2010 Andreas Kirsch]
	case 'Z':
		{
			// use the state of the current recorder to decide whether to play or stop everything
			if (m_state == REC_STATE_PLAYING_BACK_PAUSED) {
				for( List::iterator recorder = s_recorders.begin() ; recorder != s_recorders.end() ; recorder++ ) {
					(*recorder)->ContinuePlayingBack();
				}
			}
			else if (m_state == REC_STATE_PLAYING_BACK) {
				for( List::iterator recorder = s_recorders.begin() ; recorder != s_recorders.end() ; recorder++ ) {
					(*recorder)->PausePlayingBack();
				}
			}
			break;
		}
	}
}