#pragma once

#include <string>
#include <vector>

#include "Vec.h"

#include "global.h"

struct FileDesc
{
	uint			m_uiFileID;
	std::string		m_ptcFileName;
};

struct TreeDesc
{
	int				m_iX, m_iY;
	float			m_fRootMinHeight;
	float			m_fRootMaxHeight;
	uint			m_uiFileID;
	llong			m_i64FilePtr;
	uint			m_uiRootSize;
};


struct StartLocation {
	RawVec3f vStartPosition;
	float	 fStartHeading, fStartPitch;

	StartLocation() {
		vStartPosition[ 0 ] = 0.0f;
		vStartPosition[ 1 ] = 0.0f;
		vStartPosition[ 2 ] = 0.0f;
		fStartHeading = 0.0f;
		fStartPitch = 0.0f;
	}
};

class TerrainConfiguration
{
public:
	TerrainConfiguration();

	bool ReadFile(const std::string& strFileName);

	TEXTURE_COMPRESSION				GetTextureCompression() const;
	float							GetLevelZeroTileExtent() const;
	uint							GetNumLevels() const;
	const std::vector<FileDesc>&	GetFileDescs() const;
	const std::vector<TreeDesc>&	GetTreeDescs() const;
	const std::string&				GetOverviewMapFileName() const;
	void							GetOverviewMapExtents(float* pfX0, float* pfY0, float* pfExtentX, float* pfExtentY) const;
	float							GetMinVerticalTolerance() const;
	const std::string&				GetDemoFlightFileName() const;

	float							GetMaxSpeed() const;
	float							GetMinHeight() const;

	uint							GetNumStartLocations() const;
	const StartLocation&			GetStartLocation(uint ui) const;


	void							SetTextureCompression(TEXTURE_COMPRESSION eTextureCompression);
	void							SetLevelZeroTileExtent(float fLevelZeroTileExtent);
	void							SetNumLevels(uint uiNumLevels);
	void							SetFileDescs(const std::vector<FileDesc>& fileDescs);
	void							SetTreeDescs(const std::vector<TreeDesc>& treeDescs);
	void							SetOverviewMapFileName(const std::string& strOverviewMapFileName);
	void							SetOverviewMapExtents(float fX0, float fY0, float fExtentX, float fExtentY);
	void							SetMinVerticalTolerance(float fMinVerticalTolerance);
	void							SetDemoFlightFileName(const std::string& strDemoFlightFileName);

	void							SetMaxSpeed(float fMaxSpeed);
	void							SetMinHeight(float fMinHeight);

	void							SetStartLocations(const std::vector<StartLocation>& startLocations);

private:
	TEXTURE_COMPRESSION				m_eTextureCompression;
	float							m_fLevelZeroTileExtent;
	uint							m_uiNumLevels;
	std::vector<FileDesc>			m_fileDescs;
	std::vector<TreeDesc>			m_treeDescs;

	float							m_fMaxSpeed;
	float							m_fMinHeight;

	std::vector<StartLocation>      m_startLocations;

	std::string						m_strOverviewMapFileName;
	float							m_fOverviewMapX0, m_fOverviewMapY0, m_fOverviewMapExtentX, m_fOverviewMapExtentY;
	float							m_fMinVerticalTolerance;

	std::string						m_strDemoFlightFileName;

};
