#pragma once

#include <map>

namespace str {
	class LineFileMapper {
	public:
		struct LineFile {
			std::string filename;
			int line;
		};
		typedef std::map< int, LineFile, std::greater<int> > LineFileMap;
	public:
		void addMapping( int source, int target, const std::string &filename ) {
			LineFile lineFile;
			lineFile.line = target;
			lineFile.filename = filename;
			lineFileMap.insert( LineFileMap::value_type( source, lineFile ) );
		}

		LineFile mapLine( int source ) const {
			const LineFileMap::value_type &sourceTargetPair = *lineFileMap.lower_bound( source );
			LineFile lineFile = sourceTargetPair.second;
			lineFile.line += source - sourceTargetPair.first;
			return lineFile;
		}

	protected:
		LineFileMap lineFileMap;
	};
}