/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D_LIB__GLOBAL_H__
#define __TUM3D_LIB__GLOBAL_H__

#define ___STR(x) #x
#define __STR(x) ___STR(x)
#define __LOCATION__ __FILE__ ":" __STR(__LINE__)

#include "util.h"

typedef unsigned char      byte;
typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef          long long llong;
typedef unsigned long long ullong;

#define MAX_STRING_SIZE 4096 // Maximale L�nge von Strings

#define PI 3.141592654f
#define DEG2RAD( a ) float( (a) * PI / 180.f )
#define RAD2DEG( a ) float( (a) / PI * 180.f )

typedef float RawVec3f[3];
typedef float RawMat4f[4 * 4];

struct VertexBB
{
	RawVec3f Pos;
};

#define TILE_SIZE 512

#if TILE_SIZE == 256
const uint uiTILE_SIZE = 256;
const uint uiPAGE_EXTENT_IN_TILES = 8;
const uint uiNUM_XY_BITS = 9;
#endif

#if TILE_SIZE == 512
const uint uiTILE_SIZE = 512;
const uint uiPAGE_EXTENT_IN_TILES = 4;
const uint uiNUM_XY_BITS = 10;
#endif

const uint uiNUM_MIPMAP_LEVELS = FloorLog2(uiTILE_SIZE) + 1;

enum TEXTURE_COMPRESSION
{
	TEX_COMP_VQ = 0, // legacy, not supported anymore
	TEX_COMP_S3TC,
	TEX_COMP_S3TC_SHARED
};

#endif