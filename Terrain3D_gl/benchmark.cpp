/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "benchmark.h"

Benchmark::Benchmark(Camera* pCamera, const Box* pBox)
:
m_pCamera(pCamera),
m_box(*pBox),
m_state(BENCH_STATE_IDLE)
{
	//errno_t err = _wfopen_s(&m_hLogFile, L"bench.txt", L"a+t");
	//if (err != 0)
	//{
	//	TCHAR str[MAX_STRING_SIZE];
	//	_stprintf_s(str, MAX_STRING_SIZE, _T("Could not open file %s\n for appending") __LOCATION__, L"bench.txt");
	//	MessageBox(NULL, str, _T("Error"), MB_OK | MB_ICONERROR);
	//	exit(-1);
	//}
	//setvbuf(m_hLogFile, NULL, _IONBF, 0);
}


void Benchmark::Start()
{
	assert(m_state == BENCH_STATE_IDLE);

	m_pCamera->GetPosition(&m_vOriginalPosition);
	m_pCamera->GetAngles(&m_fOriginalHeading, &m_fOriginalPitch, &m_fOriginalRoll, &m_fOriginalTurn);

	m_fHeading = 180.0f;

	m_fPitch = 20.0f;

	m_state = BENCH_STATE_BENCHMARKING;
}

float RandWithinInterval(float a, float b)
{
	return a + (b - a) * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}


bool Benchmark::Update(double fTime)
{
	assert(m_state == BENCH_STATE_BENCHMARKING);

	if (m_fHeading == 180.0f)
	{
		m_fHeading = 0.0f;
		m_bToggle = false;

		m_vPosition.x = RandWithinInterval(m_box.m_vCenter.x - m_box.m_vHalfExtent.x, m_box.m_vCenter.x + m_box.m_vHalfExtent.x);
		m_vPosition.y = RandWithinInterval(m_box.m_vCenter.y - m_box.m_vHalfExtent.y, m_box.m_vCenter.y + m_box.m_vHalfExtent.y);
		m_vPosition.z = RandWithinInterval(m_box.m_vCenter.z - m_box.m_vHalfExtent.z, m_box.m_vCenter.z + m_box.m_vHalfExtent.z);
	}
	else
	{
		if (!m_bToggle)
		{
			m_bToggle = true;
		}
		else
		{
			m_fHeading += 3.0f;
			m_bToggle = false;
		}
	}

	float fHeading = m_fHeading;
	if (m_bToggle)
	{
		fHeading += 180.0f;
	}

	m_pCamera->SetPosition(&m_vPosition);
	m_pCamera->SetAngles(fHeading, m_fPitch, 0.0f, 0.0f);

	return true;
}

void Benchmark::Done()
{
	assert(m_state == BENCH_STATE_BENCHMARKING);

	m_pCamera->SetPosition(&m_vOriginalPosition);
	m_pCamera->SetAngles(m_fOriginalHeading, m_fOriginalPitch, m_fOriginalRoll, m_fOriginalTurn);

	m_state = BENCH_STATE_IDLE;
}

Benchmark::~Benchmark()
{
	//fclose(m_hLogFile);
}