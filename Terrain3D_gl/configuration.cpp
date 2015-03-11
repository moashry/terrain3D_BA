/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#include "configuration.h"

#include <string>

#include "global.h"


#ifndef EQUALIZER

CommandLine::CommandLine()
: m_tszConfigFileName(0), m_tszTerrainFileName(0)
{
}


bool CommandLine::ParseCommandLine(int argc, char **argv)
{
	if (m_tszConfigFileName != 0) { delete[] m_tszConfigFileName; m_tszConfigFileName = 0; }
	if (m_tszTerrainFileName != 0) { delete[] m_tszTerrainFileName; m_tszConfigFileName = 0; }

	for (int i = 0; i < argc; i++)
	{
		size_t length = strlen(argv[i]);
		if (length >= 4 && strcmp(argv[i] + length - 4, ".cfg") == 0 && m_tszConfigFileName == 0)
		{
			m_tszConfigFileName = new char[length + 1];
			strcpy_s(m_tszConfigFileName, (length + 1) * sizeof(char), argv[i]);
		}
		else if (length >= 8 && strcmp(argv[i] + length - 8, ".terrain") == 0 && m_tszTerrainFileName == 0)
		{
			m_tszTerrainFileName = new char[length + 1];
			strcpy_s(m_tszTerrainFileName, (length + 1) * sizeof(char), argv[i]);
		}
	}
	return true;
}


CommandLine::~CommandLine()
{
	if (m_tszConfigFileName != 0) { delete[] m_tszConfigFileName; }
	if (m_tszTerrainFileName != 0) { delete[] m_tszTerrainFileName; }
}

#endif

SystemConfiguration::SystemConfiguration()
{
	// Default-Werte setzen
	m_fZNear = 10.0f;
	m_fZFar = 200000.0f;
	m_fFovy = 45.0f;
	m_fScreenSpaceError = 0.7f;
	m_fPrefetchingFactor = 1.5f;
	m_uiViewportWidth = 640;
	m_uiViewportHeight = 480;
	m_uiInitNumVBs = 40;
	m_uiInitNumTextures = 4000;
	m_fSystemMemoryUsageLimit = 0.7f;
}


bool SystemConfiguration::ReadFile(const std::string& strFileName)
{
	FILE* hFile;
	errno_t error;
	error = fopen_s(&hFile, strFileName.c_str(), "rt");
	if (error)
	{
		printf_s("Error Opening Configuration File \"%s\"\n", strFileName.c_str());
		return false;
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
			if (strcmp(keyword, "Z_NEAR") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fZNear);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "Z_FAR") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fZFar);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "FOVY") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fFovy);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "SCREEN_SPACE_ERROR") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fScreenSpaceError);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "PREFETCHING_FACTOR") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fPrefetchingFactor);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "VIEWPORT_WIDTH") == 0)
			{
				result = sscanf_s(line, "%*s %u", &m_uiViewportWidth);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "VIEWPORT_HEIGHT") == 0)
			{
				result = sscanf_s(line, "%*s %u", &m_uiViewportHeight);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "INIT_NUM_VBS") == 0)
			{
				result = sscanf_s(line, "%*s %u", &m_uiInitNumVBs);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "INIT_NUM_TEXTURES") == 0)
			{
				result = sscanf_s(line, "%*s %u", &m_uiInitNumTextures);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "SYSTEM_MEMORY_USAGE_LIMIT") == 0)
			{
				result = sscanf_s(line, "%*s %f", &m_fSystemMemoryUsageLimit);
				bError = (result != 1);
			}
			else if (strcmp(keyword, "TERRAIN_FILE_NAME") == 0)
			{
				char str[MAX_STRING_SIZE];
				result = sscanf_s(line, "%*s %[^\t\r\n]", str, MAX_STRING_SIZE);
				m_strTerrainFileName = str;
				bError = (result != 1);
			}
			else if (strncmp(keyword, "//", 2) == 0)
			{
				// Kommentare ignorieren
			}
			else
			{
				// Kein Schl�sselwort
				bError = true;
			}
			if (bError)
			{
				printf_s("Error Parsing Configuration File \"%s\" in Line %u\n", strFileName.c_str(), uiLineNum);
				fclose(hFile);
				return false;
			}
		}

	}
	fclose(hFile);

	return true;
}

SystemConfiguration::~SystemConfiguration()
{
}

#ifdef EQUALIZER
void SystemConfiguration::getInstanceData( eq::net::DataOStream& os ) const {
	os << m_fZNear << m_fZFar
	   << m_fScreenSpaceError << m_fPrefetchingFactor
	   << m_uiInitNumVBs << m_uiInitNumTextures
	   << m_fSystemMemoryUsageLimit << m_strTerrainFileName;
	// skip fovy and viewport size (not used in equalizer)
}		

void SystemConfiguration::applyInstanceData( eq::net::DataIStream& is ) {
	is >> m_fZNear >> m_fZFar
	   >> m_fScreenSpaceError >> m_fPrefetchingFactor
	   >> m_uiInitNumVBs >> m_uiInitNumTextures
	   >> m_fSystemMemoryUsageLimit >> m_strTerrainFileName;
	// skip fovy and viewport size (not used in equalizer)
}
#endif