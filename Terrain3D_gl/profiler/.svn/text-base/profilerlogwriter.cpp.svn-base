/*************************************************************************************

Simple Runtime Profiler

Author: Marc Treib

(c) Marc Treib

mailto:treib@in.tum.de

Last Changed: 2010-04-28

*************************************************************************************/

#include "profilerlogwriter.hpp"

#include <iomanip>


ProfilerLogWriter::ProfilerLogWriter( ) {
	initFileState( );
}

ProfilerLogWriter::ProfilerLogWriter( const std::string& fileName ) : m_file( fileName.c_str( ) ) {
	initFileState( );
}

ProfilerLogWriter::~ProfilerLogWriter( ) {
	m_file.close( );
}

bool ProfilerLogWriter::open( const std::string& fileName ) {
	if( m_file.is_open( ) )
		m_file.close( );

	m_file.open( fileName.c_str( ) );

	return m_file.good( );
}

void ProfilerLogWriter::close( ) {
	m_file.close( );
}

bool ProfilerLogWriter::isOpen( ) const {
	return m_file.is_open( );
}

bool ProfilerLogWriter::isGood( ) const {
	return m_file.good( );
}

void ProfilerLogWriter::started( float timestampMS ) {
	m_file << "Profiling started at " << timestampMS << " ms\n\n";
}

void ProfilerLogWriter::stopped( float timestampMS, float profiledTimeMS, uint profiledFrames ) {
	float fps = 1000.0f * profiledFrames / profiledTimeMS;
	m_file << "Profiling stopped at " << timestampMS << " ms (profiled time: " << profiledTimeMS << " ms over " << profiledFrames << " frames, " << fps << " fps)\n\n";
}

void ProfilerLogWriter::beginOutput( float timestampMS, float profiledTimeMS, uint profiledFrames ) {
	float fps = 1000.0f * profiledFrames / profiledTimeMS;
	m_file << "Statistics at " << timestampMS << " ms (profiled time: " << profiledTimeMS << " ms over " << profiledFrames << " frames, " << fps << " fps):\n"
	       << "                       ms                      :         Count         :\n"
	       << "     Total :  Max/Call : Max/Frame :      Self :     Total : Max/Frame : Name\n"
	       << "---------------------------------------------------------------------------------------------\n";
}

void ProfilerLogWriter::putSample( const std::string& name, uint parentCount, bool hasUncountedChildren, float totalTimeMS, float maxCallTimeMS, float maxFrameTimeMS, float ownTimeMS, uint totalCount, uint maxFrameCount ) {
	m_file << " "
	       << std::setw( 9 ) << totalTimeMS << " : "
	       << std::setw( 9 ) << maxCallTimeMS << " : "
	       << std::setw( 9 ) << maxFrameTimeMS << " : "
	       << std::setw( 9 ) << ownTimeMS << " : "
	       << std::setw( 9 ) << totalCount << " : "
	       << std::setw( 9 ) << maxFrameCount << " : ";
	for( uint i = 0; i < parentCount; i++ )
		m_file << " ";
	m_file << name;
	if( hasUncountedChildren )
		m_file << " +";
	m_file << "\n";
}

void ProfilerLogWriter::endOutput( ) {
	m_file << "\n";
}

void ProfilerLogWriter::cleared( float timestampMS ) {
	m_file << "Profiler data cleared at " << timestampMS << " ms\n\n";
}

void ProfilerLogWriter::initFileState( ) {
	m_file << std::showpoint << std::right << std::fixed << std::setprecision( 1 );
}
