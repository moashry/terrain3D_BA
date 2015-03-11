/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/


#ifndef __TUM3D__CONFIGURATION_H__
#define __TUM3D__CONFIGURATION_H__

#include <string>
#include <vector>

#include "global.h"

#ifdef EQUALIZER
#	include <eq/net/dataIStream.h>
#	include <eq/net/dataOStream.h>
#endif


#ifndef EQUALIZER

class CommandLine
{
public:
	CommandLine();
	~CommandLine();
	bool ParseCommandLine(int argc, char **argv);
	bool IsConfigFileSpecified() const { return m_tszConfigFileName != 0; }
	bool IsTerrainFileSpecified() const { return m_tszTerrainFileName != 0; }
	const char* GetConfigFileName() const { return m_tszConfigFileName; }
	const char* GetTerrainFileName() const { return m_tszTerrainFileName; }
private:
	char*	m_tszConfigFileName;
	char*	m_tszTerrainFileName;
};

#endif


class SystemConfiguration
{
public:
	SystemConfiguration();
	~SystemConfiguration();

	bool ReadFile(const std::string& strFileName);

	float GetZNear() const;
	float GetZFar() const;
	float GetFovy() const;
	float GetScreenSpaceError() const;
	float GetPrefetchingFactor() const;
	uint GetViewportWidth() const;
	uint GetViewportHeight() const;
	uint GetInitNumVBs() const;
	uint GetInitNumTextures() const;
	float GetSystemMemoryUsageLimit() const;
	const std::string& GetTerrainFileName() const;

#ifdef EQUALIZER
	void getInstanceData( eq::net::DataOStream& os ) const;
	void applyInstanceData( eq::net::DataIStream& is );
#endif

private:
	float			m_fZNear;
	float			m_fZFar;
	float			m_fFovy;
	float			m_fScreenSpaceError;
	float			m_fPrefetchingFactor;
	uint			m_uiViewportWidth;
	uint			m_uiViewportHeight;
	uint			m_uiInitNumVBs;
	uint			m_uiInitNumTextures;
	float			m_fSystemMemoryUsageLimit;
	std::string		m_strTerrainFileName;
};

inline float SystemConfiguration::GetZNear() const
{
	return m_fZNear;
}

inline float SystemConfiguration::GetZFar() const
{
	return m_fZFar;
}

inline float SystemConfiguration::GetFovy() const
{
	return m_fFovy;
}

inline float SystemConfiguration::GetScreenSpaceError() const
{
	return m_fScreenSpaceError;
}

inline float SystemConfiguration::GetPrefetchingFactor() const
{
	return m_fPrefetchingFactor;
}

inline uint SystemConfiguration::GetViewportWidth() const
{
	return m_uiViewportWidth;
}

inline uint SystemConfiguration::GetViewportHeight() const
{
	return m_uiViewportHeight;
}

inline uint SystemConfiguration::GetInitNumVBs() const
{
	return m_uiInitNumVBs;
}

inline uint SystemConfiguration::GetInitNumTextures() const
{
	return m_uiInitNumTextures;
}

inline float SystemConfiguration::GetSystemMemoryUsageLimit() const
{
	return m_fSystemMemoryUsageLimit;
}

inline const std::string& SystemConfiguration::GetTerrainFileName() const
{
	return m_strTerrainFileName;
}


#endif
