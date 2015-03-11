/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/

#ifndef __TUM3D__UTIL_H__
#define __TUM3D__UTIL_H__

#include "debug.h"

#define SAFE_DELETE(p) do { delete (p); (p) = 0; } while( false )
#define V_RETURN(c)	do { if( !(c) ) { outdbg( #c " failed\n" ); return false; } } while( false )

// Liefert ceil(log_2(x)) f�r x >= 1, 0 f�r x = 0
inline unsigned int CeilLog2(unsigned int ui)
{
	unsigned int uiRes = 1;
	unsigned int uiLog = 0;
	// Es gilt stets uiRes = 2^uiLog

	// 2^(uiLog-1) = 1/2 < ui
	while (uiRes < ui)
	{
		// 2^uiLog = uiRes < ui
		uiRes <<= 1;
		uiLog++;
		// 2^(uiLog-1) < ui
	}
	// 2^(uiLog-1) < ui <= uiRes = 2^uiLog
	// uiLog-1 < log_2(ui) <= uiLog
	// uiLog = ceil(log_2(ui))

	return uiLog;
}

// Liefert floor(log_2(x)) f�r x >= 1, 0 f�r x = 0
inline unsigned int FloorLog2(unsigned int ui)
{
	unsigned int uiRes = 2;
	unsigned int uiLog = 0;
	// Es gilt stets uiRes = 2^(uiLog+1)
	
	// 2^uiLog = 1 <= ui
	while (uiRes <= ui)
	{
		// 2^(uiLog+1) = uiRes <= ui
		uiRes <<= 1;
		uiLog++;
		// 2^uiLog <= ui
	}
	// 2^uiLog <= ui < uiRes = 2^(uiLog+1)
	// uiLog <= log_2(ui) < uiLog+1
	// uiLog = floor(log_2(ui))

	return uiLog;
}


// Liefert auch true f�r ui == 0
inline bool IsPowerOfTwo(unsigned int ui)
{
	return (ui & (ui - 1)) == 0;
}

inline float sgn(float x)
{
	if (x > 0.0f) { return 1.0f; }
	else if (x < 0.0f) { return -1.0f; }
	else { return 0.0f; }
}

#endif