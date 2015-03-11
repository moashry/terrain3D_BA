/*************************************************************************************

Terrain3D

Author: Christian Dick

(c) Christian Dick

mailto:dick@in.tum.de

*************************************************************************************/


#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "global.h"

void outdbg(const char* fmt, ...)
{
//#ifdef _DEBUG
#ifndef DEMO
	if (!fmt) return;
	char text[MAX_STRING_SIZE];
	va_list ap;
	va_start(ap, fmt);
	vsprintf_s(text, MAX_STRING_SIZE, fmt, ap);
	va_end(ap);
	OutputDebugString(text);
#endif
//#endif
}
