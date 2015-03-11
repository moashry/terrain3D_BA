#pragma once

#include <string>

#include "global.h"
#include "dataloader.h"
#include "resourcepool.h"
#include "terrain.h"
#include "terrainConfiguration.h"


DataLoader* CreateDataLoader(const TerrainConfiguration& terrainConfiguration, float systemMemoryUsageLimit, float screenSpaceError);
