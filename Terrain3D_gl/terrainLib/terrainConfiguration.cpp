#include "terrainConfiguration.h"

#include <stdlib.h>


TerrainConfiguration::TerrainConfiguration()
{
	// Default-Werte setzen
	m_eTextureCompression = TEX_COMP_S3TC_SHARED;
	m_fLevelZeroTileExtent = 32768.0f;
	m_uiNumLevels = 10;
	m_fOverviewMapX0 = 0.0f;
	m_fOverviewMapY0 = 0.0f;
	m_fOverviewMapExtentX = 10000.0f;
	m_fOverviewMapExtentY = 10000.0f;
	m_fMinVerticalTolerance = 0.0f;

	m_fMaxSpeed = 5000.0f;
	m_fMinHeight = 50.0f;
}


bool TerrainConfiguration::ReadFile(const std::string& strFileName)
{
	m_startLocations.clear();

	FILE* hFile;
	errno_t error;
	error = fopen_s(&hFile, strFileName.c_str(), "rt");
	if (error)
	{
		printf_s("Error Opening Terrain File \"%s\"\n", strFileName.c_str());
		return false;
	}

	char path[MAX_STRING_SIZE];
	const char* ptcPosition = strrchr(strFileName.c_str(), '\\');
	if (ptcPosition != 0)
	{
		strncpy_s(path, MAX_STRING_SIZE, strFileName.c_str(), ptcPosition - strFileName.c_str() + 1);
	}
	else
	{
		path[0] = '\0';
	}

	uint uiLineNum = 0;
	char line[MAX_STRING_SIZE];
	char keyword[MAX_STRING_SIZE];
	while (true)
	{
		uiLineNum++;
		if (fgets(line, MAX_STRING_SIZE, hFile) == 0)
		{
			// EOF
			break;
		}
		int result;
		result = sscanf_s(line, "%s", keyword, MAX_STRING_SIZE);
		if (result > 0) // Leerzeilen ignorieren
		{
			bool bError = false;
			if (strcmp(keyword, "TEXTURE_COMPRESSION") == 0)
			{
				char str[MAX_STRING_SIZE];
				result = sscanf_s(line, "%*s %s", str, MAX_STRING_SIZE);
				bError = (result != 1);
				if (!bError)
				{
					if (strcmp(str, "VQ") == 0)
					{
						m_eTextureCompression = TEX_COMP_VQ;
						printf_s("ERROR: VQ texture compression is no longer supported!\n");
						bError = true;
					}
					else if (strcmp(str, "S3TC") == 0)
					{
						m_eTextureCompression = TEX_COMP_S3TC;
					}
					else if (strcmp(str, "S3TC_SHARED") == 0)
					{
						m_eTextureCompression = TEX_COMP_S3TC_SHARED;
					}
					else
					{
						bError = true;
					}
				}
			}
			else if (strcmp(keyword, "LEVEL_ZERO_TILE_EXTENT") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fLevelZeroTileExtent);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "NUM_LEVELS") == 0)
			{
				result = sscanf_s(line, "%*s %u", &m_uiNumLevels);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "MIN_VERTICAL_TOLERANCE") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fMinVerticalTolerance);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "TEXTURE_MAX_NUM_BITS") == 0)
			{
				// legacy option (was used for vq), ignore
			}
			else if (strcmp(keyword, "FILE") == 0)
			{
				FileDesc fileDesc;
				memset(&fileDesc, 0, sizeof(FileDesc));
				char str[MAX_STRING_SIZE];
				strcpy_s(str, MAX_STRING_SIZE, path);
				result = sscanf_s(line, "%*s %u %[^\t\r\n]", &fileDesc.m_uiFileID, str + strlen(path), MAX_STRING_SIZE - strlen(path));
				fileDesc.m_ptcFileName = str;
				bError = (result != 2);
				if (!bError)
				{
					m_fileDescs.push_back(fileDesc);
				}
			}
			else if (strcmp(keyword, "TREE") == 0)
			{
				TreeDesc treeDesc;
				memset(&treeDesc, 0, sizeof(TreeDesc));
				result = sscanf_s(line, "%*s %i %i %f %f %u %I64u %u",
					&treeDesc.m_iX, &treeDesc.m_iY,
					&treeDesc.m_fRootMinHeight, &treeDesc.m_fRootMaxHeight,
					&treeDesc.m_uiFileID, &treeDesc.m_i64FilePtr, &treeDesc.m_uiRootSize);
				bError = (result != 7);
				if (!bError)
				{
					m_treeDescs.push_back(treeDesc);
				}
			}
			else if (strcmp(keyword, "OVERVIEW_MAP") == 0)
			{
				char filename[MAX_STRING_SIZE];
				result = sscanf_s(line, "%*s %s", filename, MAX_STRING_SIZE);
				bError = (result != 1);
				if( !bError )
				{
					m_strOverviewMapFileName = std::string( path ) + std::string( filename );
				}
			}
			else if (strcmp(keyword, "OVERVIEW_MAP_EXTENTS") == 0)
			{
				result = sscanf_s(line, "%*s %f %f %f %f", &m_fOverviewMapX0, &m_fOverviewMapY0, &m_fOverviewMapExtentX, &m_fOverviewMapExtentY);
				bError = (result != 4);
			}
			else if (strcmp(keyword, "START_POSITION") == 0)
			{
				StartLocation currentLocation;
				result = sscanf_s(line, "%*s %f %f %f", &currentLocation.vStartPosition[0], &currentLocation.vStartPosition[1], &currentLocation.vStartPosition[2]);
				bError = (result != 3);

				m_startLocations.push_back(currentLocation);
			}
			else if (strcmp(keyword, "START_ANGLES") == 0)
			{
				StartLocation &currentLocation = m_startLocations.back();

				result = sscanf_s(line, "%*s %f %f", &currentLocation.fStartHeading, &currentLocation.fStartPitch);
			}
			else if (strcmp(keyword, "MAX_SPEED") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fMaxSpeed);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "MIN_HEIGHT") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fMinHeight);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "DEMO_FLIGHT") == 0)
			{
				char filename[MAX_STRING_SIZE];
				result = sscanf_s(line, "%*s %s", filename, MAX_STRING_SIZE);
				bError = (result != 1);
				if( !bError )
				{
					m_strDemoFlightFileName = std::string( path ) + std::string( filename );
				}
			}
			else if (strncmp(keyword, "//", 2) == 0)
			{
				// Kommentare ignorieren
			}
			else
			{
				// Kein Schlüsselwort
				bError = true;
			}
			if (bError)
			{
				printf_s("Error Parsing Terrain File \"%s\" in Line %u\n", strFileName.c_str(), uiLineNum);
				fclose(hFile);
				return false;
			}
		}

	}
	fclose(hFile);

	return true;
}

TEXTURE_COMPRESSION TerrainConfiguration::GetTextureCompression() const
{
	return m_eTextureCompression;
}

float TerrainConfiguration::GetLevelZeroTileExtent() const
{
	return m_fLevelZeroTileExtent;
}

uint TerrainConfiguration::GetNumLevels() const
{
	return m_uiNumLevels;
}

const std::vector<FileDesc>& TerrainConfiguration::GetFileDescs() const
{
	return m_fileDescs;
}

const std::vector<TreeDesc>& TerrainConfiguration::GetTreeDescs() const
{
	return m_treeDescs;
}

const std::string& TerrainConfiguration::GetOverviewMapFileName() const
{
	return m_strOverviewMapFileName;
}

void TerrainConfiguration::GetOverviewMapExtents(float* pfX0, float* pfY0, float* pfExtentX, float* pfExtentY) const
{
	*pfX0 = m_fOverviewMapX0;
	*pfY0 = m_fOverviewMapY0;
	*pfExtentX = m_fOverviewMapExtentX;
	*pfExtentY = m_fOverviewMapExtentY;
}

float TerrainConfiguration::GetMinVerticalTolerance() const
{
	return m_fMinVerticalTolerance;
}


const std::string& TerrainConfiguration::GetDemoFlightFileName() const
{
	return m_strDemoFlightFileName;
}

float TerrainConfiguration::GetMaxSpeed() const
{
	return m_fMaxSpeed;
}

float TerrainConfiguration::GetMinHeight() const
{
	return m_fMinHeight;
}

uint TerrainConfiguration::GetNumStartLocations() const
{
	return uint(m_startLocations.size());
}

const StartLocation& TerrainConfiguration::GetStartLocation(uint ui) const
{
	return m_startLocations[ui];
}


void TerrainConfiguration::SetTextureCompression(TEXTURE_COMPRESSION eTextureCompression)
{
	m_eTextureCompression = eTextureCompression;
}

void TerrainConfiguration::SetLevelZeroTileExtent(float fLevelZeroTileExtent)
{
	m_fLevelZeroTileExtent = fLevelZeroTileExtent;
}

void TerrainConfiguration::SetNumLevels(uint uiNumLevels)
{
	m_uiNumLevels = uiNumLevels;
}

void TerrainConfiguration::SetFileDescs(const std::vector<FileDesc>& fileDescs)
{
	m_fileDescs = fileDescs;
}

void TerrainConfiguration::SetTreeDescs(const std::vector<TreeDesc>& treeDescs)
{
	m_treeDescs = treeDescs;
}

void TerrainConfiguration::SetOverviewMapFileName(const std::string& strOverviewMapFileName)
{
	m_strOverviewMapFileName = strOverviewMapFileName;
}

void TerrainConfiguration::SetOverviewMapExtents(float fX0, float fY0, float fExtentX, float fExtentY)
{
	m_fOverviewMapX0 = fX0;
	m_fOverviewMapY0 = fY0;
	m_fOverviewMapExtentX = fExtentX;
	m_fOverviewMapExtentY = fExtentY;
}

void TerrainConfiguration::SetMinVerticalTolerance(float fMinVerticalTolerance)
{
	m_fMinVerticalTolerance = fMinVerticalTolerance;
}

void TerrainConfiguration::SetDemoFlightFileName(const std::string& strDemoFlightFileName)
{
	m_strDemoFlightFileName = strDemoFlightFileName;
}

void TerrainConfiguration::SetMaxSpeed(float fMaxSpeed)
{
	m_fMaxSpeed = fMaxSpeed;
}

void TerrainConfiguration::SetMinHeight(float fMinHeight)
{
	m_fMinHeight = fMinHeight;
}

void TerrainConfiguration::SetStartLocations(const std::vector<StartLocation>& startLocations)
{
	m_startLocations = startLocations;
}
