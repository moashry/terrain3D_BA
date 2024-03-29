#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string>

inline std::string strvprintf( const char *fmt, va_list args ) {
	const int maxSize = 4096;
	char output[maxSize];

	vsnprintf_s( output, maxSize, _TRUNCATE, fmt, args );

	return std::string( output );
}

inline std::string strprintf( const char *fmt, ... ) {
	va_list list;
	va_start( list, fmt);
	std::string output = strvprintf( fmt, list );
	va_end( list );

	return output;
}

