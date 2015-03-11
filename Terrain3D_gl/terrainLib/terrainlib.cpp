#include "terrainlib.h"

#include <windows.h> // for GlobalMemoryStatusEx

#include "terrain.h"
#include "resourcepool.h"
#include "dataloader.h"

#include "util.h"

#define MAX_MEMORY_32   (1584 << 20)


DataLoader* CreateDataLoader(const TerrainConfiguration& terrainConfiguration, float systemMemoryUsageLimit, float screenSpaceError)
{
	// Berechne wie viel Systemspeicher vom Data Loader verwendet werden darf
	MEMORYSTATUSEX memoryStatusEx;
	ZeroMemory(&memoryStatusEx, sizeof(MEMORYSTATUSEX));
	memoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
	BOOL bResult = GlobalMemoryStatusEx(&memoryStatusEx);
	if (!bResult)
	{
		printf_s("GlobalMemoryStatusEx failed");
		exit(-1);
	}

	// Minimum muss bestimmt werden, da der vorhandene physikalische Speicher gr��er als der User-Mode-Bereich des Adressraums des Prozesses sein k�nnte
	llong systemMemoryLimit = std::min(
		static_cast<llong>(systemMemoryUsageLimit * static_cast<float>(memoryStatusEx.ullAvailPhys)),
		static_cast<llong>(0.8f * static_cast<float>(memoryStatusEx.ullAvailVirtual))
		);
#ifndef _M_X64
	systemMemoryLimit = std::min(systemMemoryLimit, (llong) MAX_MEMORY_32);
#endif

	printf_s( "Available Physical Memory: %.2f MB\n", static_cast<float>(memoryStatusEx.ullAvailPhys) / (1024.0f * 1024.0f));
	printf_s( "Available Virtual Memory:  %.2f MB\n", static_cast<float>(memoryStatusEx.ullAvailVirtual) / (1024.0f * 1024.0f));
	printf_s( "Dataloader Limit:          %.2f MB\n", static_cast<float>(systemMemoryLimit) / (1024.0f * 1024.0f));

	return new DataLoader(
		terrainConfiguration.GetLevelZeroTileExtent(), terrainConfiguration.GetNumLevels(),
		terrainConfiguration.GetFileDescs(), terrainConfiguration.GetTreeDescs(),
		200, systemMemoryLimit);
}